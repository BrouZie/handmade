#include "sdl_handmade.h"

#include "handmade.cpp" // -- unity build
#include "handmade.h"

/*
    TODO: THIS IS NOT A FINILIZED PLATFORM API LAYER!

    - Saved game locations
    - Getting a handle to our own executable file
    - Asset loading path
    - Threading (launch a thread)
    - Raw input (support for multiple keyboards)
    - Sleep/timeBeginPeriod
    - ClipCursor() (multimonitor support)
    - Fullscreen support
    - WM_ACTIVATEAPP (NOT FOCUSED FOR LINUX I GUESS)
    - Resize or render optimizations or something
    - Hardware acceleration - (opengl or vulkan maybe)
    - International WASD support
 */

enum ToneInterval
{
    C0 = 0,
    Db = 1,
    D  = 2,
    Eb = 3,
    E  = 4,
    F  = 5,
    Gb = 6,
    G  = 7,
    Ab = 8,
    A  = 9,
    Bb = 10,
    H  = 11,
    C1 = 12
};

// RENDERING
global_variable SDLOffscreenBuffer GlobalBackbuffer;

// AUDIO
global_variable SDLAudioRingBuffer AudioRingBuffer;

// EXPERIMENTAL AUDIO
global_variable ToneInterval TargetToneInterval;

// GAME INPUT
#define MAX_CONTROLLERS 4
global_variable SDL_GameController* ControllerHandles[MAX_CONTROLLERS];
global_variable SDL_Haptic*         RumbleHandles[MAX_CONTROLLERS];

// TODO: Kept to test keyboard input - remove later
global_variable int LineHeight = 10;
global_variable int LineWidth  = 40;

global_variable int StartRow;
global_variable int StartCol;

inline SDLWindowDimensions get_window_dimensions(SDL_Window* window)
{
	SDLWindowDimensions dimensions;
    SDL_GetWindowSize(window, &dimensions.width, &dimensions.height);
	return dimensions;
}

// Currently reliant upon global variables
internal void render_weird_rectangleshape(SDLOffscreenBuffer* buffer)
{
    int width  = buffer->width;
    int height = buffer->height;

	uint8_t Blue  = 0;
	uint8_t Green = 50;
	uint8_t Red   = 205;

    uint8_t* start_pos = (uint8_t *)buffer->memory + (buffer->pitch * StartRow) + (4 * StartCol);
    for(int y {}; y < LineHeight; ++y)
	{
        uint32_t *pixel = (uint32_t *)start_pos;
        for(int x {}; x < LineWidth; ++x)
		{
            *pixel++ = (Red << 16) | (Green << 8) | Blue;
        }
        start_pos += buffer->pitch;
    }
}

internal void resize_texture(SDLOffscreenBuffer* buffer, SDL_Renderer* renderer, int width, int height)
{
	int bytes_per_pixel = 4;
    if (buffer->memory)
    {
        munmap(buffer->memory,
               buffer->width * buffer->height * bytes_per_pixel);
    }

    if (buffer->texture)
    {
        SDL_DestroyTexture(buffer->texture);
    }

    buffer->texture = SDL_CreateTexture(renderer,
										SDL_PIXELFORMAT_ARGB8888,
										SDL_TEXTUREACCESS_STREAMING,
										width,
										height);
    buffer->width  = width;
    buffer->height = height;
    buffer->pitch  = width * bytes_per_pixel;
    buffer->memory = mmap(nullptr,
						  buffer->width * buffer->height * bytes_per_pixel,
						  PROT_READ | PROT_WRITE,
						  MAP_ANONYMOUS | MAP_PRIVATE,
						  -1,
						  0);

	// TODO: Probably clear this to black
}

internal void sdl_update_window(SDLOffscreenBuffer* buffer, SDL_Window* window, SDL_Renderer* renderer)
{
	// TODO: Aspect ratio correction
	SDL_UpdateTexture(buffer->texture, nullptr, buffer->memory, buffer->pitch);
	SDL_RenderCopy   (renderer, buffer->texture, nullptr, nullptr);
   	SDL_RenderPresent(renderer);
}

internal void sdl_fill_sound_buffer(SDLSoundOutput *sound_output, int byte_to_lock, int bytes_to_write, GameSoundOutputBuffer* sound_buffer)
{
    int16_t *samples = sound_buffer->samples;
    void *region1 = (uint8_t*)AudioRingBuffer.data + byte_to_lock;
    int region1_size = bytes_to_write;
    if (region1_size + byte_to_lock > sound_output->secondary_buffer_size)
    {
        region1_size = sound_output->secondary_buffer_size - byte_to_lock;
    }
    void *region2 = AudioRingBuffer.data;
    int region2_size = bytes_to_write - region1_size;
    int region1_sample_count = region1_size/sound_output->bytes_per_sample;
    int16_t *sample_out = (int16_t *)region1;
    for(int sample_idx = 0; sample_idx < region1_sample_count; ++sample_idx)
    {
        *sample_out++ = *samples++;
        *sample_out++ = *samples++;

        ++sound_output->running_sample_idx;
    }

    int region2_sample_count = region2_size/sound_output->bytes_per_sample;
    sample_out = (int16_t *)region2;
    for(int sample_idx = 0; sample_idx < region2_sample_count; ++sample_idx)
    {
        *sample_out++ = *samples++;
        *sample_out++ = *samples++;
        ++sound_output->running_sample_idx;
    }
}

bool32 event_callback(SDLOffscreenBuffer* buffer, SDL_Event* event)
{
	bool32 terminate_app = false;
    switch (event->type)
    {
		case SDL_QUIT:
		{
            terminate_app = true;
		    printf("SDL_QUIT\n");
		} break;

        // TODO: Add SDL_KEYDOWN somewhere?
		case SDL_KEYDOWN:
        {
            SDL_Keycode key_code = event->key.keysym.sym;

            // NOTE: In the windows version, we used "if (IsDown != WasDown)"
            // to detect key repeats. SDL has the 'repeat' value, though,
            // which we'll use.
            if (event->key.repeat == 0)
            {
                switch(key_code)
                {
                    case SDLK_UP:
                    {
                        LineHeight -= 5;
                    } break;
                    case SDLK_LEFT:
                    {
                        LineWidth -= 5;
                    } break;
                    case SDLK_DOWN:
                    {
                        LineHeight += 5;
                    } break;
                    case SDLK_RIGHT:
                    {
                        LineWidth += 5;
                    } break;
                    case SDLK_w:
                    {
                        StartRow -= 5;
                    } break;
                    case SDLK_a:
                    {
                        StartCol -= 5;
                        TargetToneInterval = C0;
                    } break;
                    case SDLK_s:
                    {
                        StartRow += 5;
                        TargetToneInterval = Db;
                    } break;
                    case SDLK_d:
                    {
                        StartCol += 5;
                        TargetToneInterval = D;
                    } break;
                    case SDLK_f:
                    {
                        TargetToneInterval = Eb;
                    } break;
                    case SDLK_g:
                    {
                        TargetToneInterval = E;
                    } break;
                    case SDLK_y:
                    {
                        TargetToneInterval = F;
                    } break;
                    case SDLK_h:
                    {
                        TargetToneInterval = Gb;
                    } break;
                    case SDLK_u:
                    {
                        TargetToneInterval = G;
                    } break;
                    case SDLK_j:
                    {
                        TargetToneInterval = Ab;
                    } break;
                    case SDLK_i:
                    {
                        TargetToneInterval = A;
                    } break;
                    case SDLK_k:
                    {
                        TargetToneInterval = Bb;
                    } break;
                    case SDLK_o:
                    {
                        TargetToneInterval = H;
                    } break;
                    case SDLK_l:
                    {
                        TargetToneInterval = C1;
                    } break;
                }
            }
            bool32 alt_key_was_down = (event->key.keysym.mod & KMOD_ALT);
            if((key_code == SDLK_F4) && alt_key_was_down)
            {
                terminate_app = true;
            }
        } break;

		case SDL_WINDOWEVENT:
		{
			switch(event->window.event)
			{
				case SDL_WINDOWEVENT_SIZE_CHANGED: // this event happens twice on startup
				{
                    printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", event->window.data1, event->window.data2);
				} break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				{
                    printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
				} break;

				// Equivalent to Casey's WM_PAINT case
				case SDL_WINDOWEVENT_EXPOSED:
				{
					// local_persist bool32 is_white = true;
					SDL_Window* window   = SDL_GetWindowFromID(event->window.windowID);
					SDL_Renderer* renderer = SDL_GetRenderer(window);
					sdl_update_window(buffer, window, renderer);
				} break;
			}
		} break;
    }
    return terminate_app;
}

internal void sdl_open_game_controllers()
{
	int all_joysticks = SDL_NumJoysticks();
	int ctrl_idx = 0;
	for (int stick_idx{}; stick_idx < all_joysticks; ++stick_idx)
	{
		if (!SDL_IsGameController(stick_idx))
		{
			continue;
		}
		if (ctrl_idx >= MAX_CONTROLLERS)
		{
			break;
		}
		ControllerHandles[ctrl_idx] = SDL_GameControllerOpen(stick_idx);
		RumbleHandles[ctrl_idx]     = SDL_HapticOpen(stick_idx);

		if (RumbleHandles[ctrl_idx] && SDL_HapticRumbleInit(RumbleHandles[ctrl_idx]) != 0)
		{
			SDL_HapticClose(RumbleHandles[ctrl_idx]);
            RumbleHandles[ctrl_idx] = 0;
		}
		++ctrl_idx;
	}
    if (!all_joysticks)
    {
        printf("NO JOYSTICKS DETECTED\n");
    }
}

internal void sdl_close_game_controllers()
{
    for(int ctrl_idx = 0; ctrl_idx < MAX_CONTROLLERS; ++ctrl_idx)
    {
        // NOTE: Touching globals
        if (ControllerHandles[ctrl_idx])
        {
            if (RumbleHandles[ctrl_idx])
            {
                SDL_HapticClose(RumbleHandles[ctrl_idx]);
            }
            SDL_GameControllerClose(ControllerHandles[ctrl_idx]);
        }
    }
}

internal void sdl_process_game_controller_button(GameButtonState* old_state,
                                                 GameButtonState* new_state,
                                                 SDL_GameController* controller_handle,
                                                 SDL_GameControllerButton button)
{
    new_state->ended_down = SDL_GameControllerGetButton(controller_handle, button);
    new_state->half_transition_count += ((new_state->ended_down == old_state->ended_down) ? 0 : 1);
}

// This is where the audio memory is populated
internal void sdl_audio_callback(void* user_data, Uint8* audio_data, int length)
{
    SDLAudioRingBuffer* ring_buffer = (SDLAudioRingBuffer*)user_data;

    int region1_size = length;
    int region2_size = 0;
    if (ring_buffer->play_cursor + length > ring_buffer->size)
    {
        region1_size = ring_buffer->size - ring_buffer->play_cursor;
        region2_size = length - region1_size;
    }
    memcpy(audio_data, (uint8_t*)(ring_buffer->data) + ring_buffer->play_cursor, region1_size);
    memcpy(&audio_data[region1_size], ring_buffer->data, region2_size);
    ring_buffer->play_cursor = (ring_buffer->play_cursor + length) % ring_buffer->size;
    ring_buffer->write_cursor = (ring_buffer->play_cursor + 2048) % ring_buffer->size;
}

internal void sdl_init_audio(int32_t samples_per_second, int32_t buffer_size)
{
    SDL_AudioSpec AudioSettings { };

    AudioSettings.freq     = samples_per_second;
    AudioSettings.format   = AUDIO_S16LSB;
    AudioSettings.channels = 2;
    AudioSettings.samples  = 512;
    AudioSettings.callback = &sdl_audio_callback;

    AudioSettings.userdata = &AudioRingBuffer; // NOTE: GLOBAL VARIABLE

    // NOTE: Touching GLOBALS
    AudioRingBuffer.data = malloc(buffer_size);
    AudioRingBuffer.size = buffer_size;
    AudioRingBuffer.write_cursor = 0;
    AudioRingBuffer.play_cursor = 0;

    SDL_OpenAudio(&AudioSettings, 0);

    printf("Initialised an Audio device at frequency %d Hz, %d Channels, buffer size %d\n",
           AudioSettings.freq, AudioSettings.channels, AudioSettings.samples);

    if (AudioSettings.format != AUDIO_S16)
    {
        printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
        SDL_CloseAudio();
    }
}

// MY GUESS IS:

// game_init(GameOffscreenBuffer& buffer); SUCCESS/ERROR
//     Initializes: SDL stuff -> window, renderer, audio
//     Handles events: not sure exactly how, but it does okey?
//
// game_update(); -> SUCCESS/ERROR
//     One pass: everything inside the while loop
//     NB: I think the loop itself is surfaced to user

// Example API implementation:

// int main()
// {
//     GameOffscreenBuffer buffer {...};
//
//     game_init(&buffer);
//
//     while (true)
//     {
//          transform_stuff(&things);
//          ...
//
//          game_update(&state_of_all_things);
//     }
//
//     return 0;
// }

int main(int argc, char* argv[])
{
    SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO );
    SDL_Window* window = SDL_CreateWindow("My SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2000, 800,
                                          SDL_WINDOW_RESIZABLE);
	sdl_open_game_controllers();

    if (window)
    {
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer)
        {
			SDLWindowDimensions Dimensions = get_window_dimensions(window);
            resize_texture(&GlobalBackbuffer, renderer, Dimensions.width, Dimensions.height);

            // AUDIO SETUP
            SDLSoundOutput SoundOutput {};
            SoundOutput.samples_per_second = 48000;
            SoundOutput.tone_volume = 3000;
            SoundOutput.running_sample_idx = 0;
            SoundOutput.wave_period = SoundOutput.samples_per_second / 256;
            SoundOutput.bytes_per_sample = sizeof(int16_t) * 2;
            SoundOutput.secondary_buffer_size = SoundOutput.samples_per_second * SoundOutput.bytes_per_sample;
            SoundOutput.t_sine = 0.0f;
            SoundOutput.latency_sample_count = SoundOutput.samples_per_second / 15;
            // Open our audio device:
            sdl_init_audio(SoundOutput.samples_per_second, SoundOutput.secondary_buffer_size);
            // NOTE: calloc() allocates memory and clears it to zero. It accepts the number of things being allocated and their size.
            // int16_t Samples[48000]; // STACK SMASHING
            int16_t *Samples = (int16_t *)calloc(SoundOutput.samples_per_second, SoundOutput.bytes_per_sample);
            SDL_PauseAudio(0);

            GameInput Input[2] = {};
            GameInput* NewInput = &Input[0];
            GameInput* OldInput = &Input[1];

            // NOTE: PROFILING
#if PROFILING_ON
            uint64_t perf_count_freq = SDL_GetPerformanceFrequency();
            uint64_t last_counter = SDL_GetPerformanceCounter();
            uint64_t last_cycle_count  = _rdtsc();
#endif

			bool32 Running = true;

			while (Running)
            {

                SDL_Event event;
                while (SDL_PollEvent(&event))
				{
					if (event_callback(&GlobalBackbuffer, &event))
					{
						Running = false;
					}
				}


				for (int ctrl_idx{}; ctrl_idx < MAX_CONTROLLERS; ++ctrl_idx)
				{
					if (ControllerHandles[ctrl_idx] != 0 && SDL_GameControllerGetAttached(ControllerHandles[ctrl_idx]))
					{
                        GameControllerInput* old_controller = &OldInput->controllers[ctrl_idx];
                        GameControllerInput* new_controller = &NewInput->controllers[ctrl_idx];

						sdl_process_game_controller_button(&(old_controller->up),
                                                           &(new_controller->up),
                                                           ControllerHandles[ctrl_idx],
                                                           SDL_CONTROLLER_BUTTON_DPAD_UP);
						sdl_process_game_controller_button(&(old_controller->down),
                                                           &(new_controller->down),
                                                           ControllerHandles[ctrl_idx],
                                                           SDL_CONTROLLER_BUTTON_DPAD_DOWN);
						sdl_process_game_controller_button(&(old_controller->left),
                                                           &(new_controller->left),
                                                           ControllerHandles[ctrl_idx],
                                                           SDL_CONTROLLER_BUTTON_DPAD_LEFT);
						sdl_process_game_controller_button(&(old_controller->right),
                                                           &(new_controller->right),
                                                           ControllerHandles[ctrl_idx],
                                                           SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
						// bool32 start      = sdl_process_game_controller_button(&(old_controller->left_shoulder),
						//                                                                          &(new_controller->left_shoulder),
						//                                                                          ControllerHandles[ctrl_idx],
						//                                                                          SDL_CONTROLLER_BUTTON_START);
						// bool32 back       = sdl_process_game_controller_button(&(old_controller->back),
						//                                                                          &(new_controller->back),
						//                                                                          ControllerHandles[ctrl_idx],
						//                                                                          SDL_CONTROLLER_BUTTON_BACK);
						sdl_process_game_controller_button(&(old_controller->left_shoulder),
                                                           &(new_controller->left_shoulder),
                                                           ControllerHandles[ctrl_idx],
                                                           SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
                        sdl_process_game_controller_button(&(old_controller->right_shoulder),
                                                           &(new_controller->right_shoulder),
                                                           ControllerHandles[ctrl_idx],
                                                           SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
						// bool32 a_button   = sdl_process_game_controller_button(&(old_controller->left_shoulder),
						//                                                                          &(new_controller->left_shoulder),
						//                                                                          ControllerHandles[ctrl_idx],
						//                                                                          SDL_CONTROLLER_BUTTON_A);
						// bool32 b_button   = sdl_process_game_controller_button(&(old_controller->left_shoulder),
						//                                                                          &(new_controller->left_shoulder),
						//                                                                          ControllerHandles[ctrl_idx],
						//                                                                          SDL_CONTROLLER_BUTTON_B);
						// bool32 x_button   = sdl_process_game_controller_button(&(old_controller->left_shoulder),
						//                                                                          &(new_controller->left_shoulder),
						//                                                                          ControllerHandles[ctrl_idx],
						//                                                                          SDL_CONTROLLER_BUTTON_X);
						// bool32 y_button   = sdl_process_game_controller_button(&(old_controller->is_analog),
						//                                                                          &(new_controller->left_shoulder),
						//                                                                          ControllerHandles[ctrl_idx],
						//                                                                          SDL_CONTROLLER_BUTTON_Y);

                        new_controller->start_x = old_controller->end_x;
                        new_controller->start_y = old_controller->end_y;

						int16_t stick_x = SDL_GameControllerGetAxis(ControllerHandles[ctrl_idx], SDL_CONTROLLER_AXIS_LEFTX);
						int16_t stick_y = SDL_GameControllerGetAxis(ControllerHandles[ctrl_idx], SDL_CONTROLLER_AXIS_LEFTY);

                        if (stick_x < 0)
                        {
                            new_controller->end_x = stick_x / 32768.0f;
                        }
                        else
                        {
                            new_controller->end_x = stick_x / 32767.0f;
                        }

                        new_controller->min_x = new_controller->max_x = new_controller->end_x;

                        if (stick_y < 0)
                        {
                            new_controller->end_y = stick_y / 32768.0f;
                        }
                        else
                        {
                            new_controller->end_y = stick_y / 32767.0f;
                        }
                        new_controller->min_y = new_controller->max_y = new_controller->end_y;

						// if (b_button)
						// {
						// 	if (RumbleHandles[ctrl_idx])
						// 	{
						// 		SDL_HapticRumblePlay(RumbleHandles[ctrl_idx], 0.5f, 100);
						// 	}
						// }
					}
					else
					{
						// No gamepads plugged in
					}
				}

                // Sound output test
                SDL_LockAudio();
                int byte_to_lock = (SoundOutput.running_sample_idx*SoundOutput.bytes_per_sample) % SoundOutput.secondary_buffer_size;
                int target_cursor = ((AudioRingBuffer.play_cursor +
                                        (SoundOutput.latency_sample_count*SoundOutput.bytes_per_sample)) %
                                        SoundOutput.secondary_buffer_size);
                int bytes_to_write;
                if (byte_to_lock > target_cursor)
                {
                    bytes_to_write = SoundOutput.secondary_buffer_size - byte_to_lock;
                    bytes_to_write += target_cursor;
                }
                else
                {
                    bytes_to_write = target_cursor - byte_to_lock;
                }
                SDL_UnlockAudio();

                GameOffscreenBuffer game_offscreen_buffer = {
                    .memory = GlobalBackbuffer.memory,
                    .width  = GlobalBackbuffer.width,
                    .height = GlobalBackbuffer.height,
                    .pitch  = GlobalBackbuffer.pitch,
                };
                GameSoundOutputBuffer game_sound_output_buffer = {
                    .samples_per_second = SoundOutput.samples_per_second,
                    .sample_count       = bytes_to_write / SoundOutput.bytes_per_sample,
                    .samples            = Samples
                };
                game_update(&game_offscreen_buffer, &game_sound_output_buffer, NewInput);

				// render_weird_rectangleshape(&GlobalBackbuffer);
                sdl_fill_sound_buffer(&SoundOutput, byte_to_lock, bytes_to_write, &game_sound_output_buffer);
				sdl_update_window(&GlobalBackbuffer, window, renderer);

                GameInput* tmp = NewInput;
                NewInput = OldInput;
                OldInput = tmp;

                printf("length or something of Input: %zu\n", ARRAY_COUNT(Input));

#if PROFILING_ON
                // Building profiling information
                uint64_t end_counter = SDL_GetPerformanceCounter();
                uint64_t counter_elapsed = end_counter - last_counter;

                real32 ms_per_frame = (((1000.0f * (real64)counter_elapsed / (real64)perf_count_freq)));
                real32 fps = (real64)perf_count_freq / (real64)counter_elapsed;

                uint64_t end_cycle_count = _rdtsc();
                uint64_t cycles_elapsed = end_cycle_count - last_cycle_count;
                real64   mcpf = ((real64)cycles_elapsed / (1000.0f * 1000.0f));

                printf("%.02fms/f, %.02ff/s, %.02fmc/f\n", ms_per_frame, fps, mcpf);
                last_cycle_count = end_cycle_count;
                last_counter = end_counter;
#endif
            }
        }
    }

    sdl_close_game_controllers();
    SDL_Quit();

    return 0;
}
