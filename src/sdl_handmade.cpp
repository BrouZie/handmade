#include "fft.cpp" // REMOVE ME PLEASE

#include <SDL2/SDL.h>
#include <sys/mman.h>

#define internal static
#define global_variable static
#define local_persist static

#define SAMPLES_PER_SECOND 48000
#define AUDIO_SAMPLES 512
#define BASE_TONE_FREQUENCY 261.626f

typedef int32_t bool32;

struct OffscreenBuffer
{
	// NOTE: Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
	SDL_Texture* texture;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct AudioSettings
{
    int    samples_per_second;
    int    dev;
    Uint16 buffer_size;
};

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
global_variable OffscreenBuffer GlobalBackbuffer;

// AUDIO
global_variable float Phase;   // keep this between buffers - consider adding to AudioSettings or something
global_variable ToneInterval TargetToneInterval;
global_variable float AudioData[AUDIO_SAMPLES];

#define MAX_CONTROLLERS 4

// GAME INPUT
global_variable SDL_GameController* ControllerHandles[MAX_CONTROLLERS];
global_variable SDL_Haptic*         RumbleHandles[MAX_CONTROLLERS];

// TODO: Kept to test keyboard input - remove later
global_variable int LineHeight = 10;
global_variable int LineWidth  = 40;

global_variable int StartRow;
global_variable int StartCol;

struct WindowDimensions // only used as a helper, not enforced throughout
{
	int width;
	int height;
};

inline WindowDimensions get_window_dimensions(SDL_Window* window)
{
	WindowDimensions dimensions;
    SDL_GetWindowSize(window, &dimensions.width, &dimensions.height);
	return dimensions;
}

internal void render_weird_gradient(OffscreenBuffer* buffer, int BlueOffset, int GreenOffset)
{
    int width  = buffer->width;
    int height = buffer->height;

    uint8_t* row = (uint8_t *)buffer->memory;
    for(int y = 0; y < buffer->height; ++y)
	{
        uint32_t *pixel = (uint32_t *)row;
        for(int x {}; x < buffer->width; ++x)
		{
            uint8_t Blue = (x + BlueOffset);
            uint8_t Green = (y + GreenOffset);

            *pixel++ = ((Green << 8) | Blue);
        }

        row += buffer->pitch;
    }
}

// Currently reliant upon global variables
internal void render_weird_rectangleshape(OffscreenBuffer* buffer)
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

internal void resize_texture(OffscreenBuffer* buffer, SDL_Renderer* renderer, int width, int height)
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

internal void display_buf_in_window(OffscreenBuffer* buffer, SDL_Window* window, SDL_Renderer* renderer)
{
	// TODO: Aspect ratio correction
	SDL_UpdateTexture(buffer->texture, nullptr, buffer->memory, buffer->pitch);
	SDL_RenderCopy   (renderer, buffer->texture, nullptr, nullptr);
   	SDL_RenderPresent(renderer);
}

void set_audio_tone(float* user_data, float tone_hz, int size)
{
    float phase_increment = 2.0f * (float)M_PI * tone_hz / SAMPLES_PER_SECOND;

    for (int i = 0; i < size; ++i)
    {
        user_data[i] = 0.25f * sinf(Phase);
        Phase += phase_increment;

        if (Phase >= 2.0f * (float)M_PI)
        {
            Phase -= 2.0f * (float)M_PI;
        }
    }
}

bool event_callback(OffscreenBuffer* buffer, SDL_Event* event)
{
	bool terminate_app = false;
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
            bool alt_key_was_down = (event->key.keysym.mod & KMOD_ALT);
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
					// local_persist bool is_white = true;
					SDL_Window* window   = SDL_GetWindowFromID(event->window.windowID);
					SDL_Renderer* renderer = SDL_GetRenderer(window);
					display_buf_in_window(buffer, window, renderer);
				} break;
			}
		} break;
    }
    return terminate_app;
}

internal void open_game_controllers()
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

// This is where the audio memory is populated
internal void audio_callback(void* user_data, Uint8* audio_data, int length)
{
    // Clear audio buffer to silence
    memset(audio_data, 0, length);
    memcpy(audio_data, user_data, length);
}

internal void open_audio_device(AudioSettings* audio_settings, void* user_data)
{
    FFT_init(); // NOTE: Might be useful as a debug thing
    SDL_AudioSpec AudioSettings { };

    AudioSettings.freq     = SAMPLES_PER_SECOND;
    AudioSettings.format   = AUDIO_F32;
    AudioSettings.channels = 2;
    AudioSettings.samples  = audio_settings->buffer_size;
    AudioSettings.callback = *audio_callback;

    AudioSettings.userdata = user_data;

    SDL_OpenAudio(&AudioSettings, 0);
    SDL_PauseAudio(0); // Unpause audio

    if (AudioSettings.format != AUDIO_S16)
    {
        ; // TODO: Complain if we can't get an S16L buffer.
    }
}

internal void other_audio(AudioSettings* audio_settings)
{
    FFT_init(); // NOTE: Might be useful as a debug thing
    SDL_AudioSpec desired {};
    desired.freq = SAMPLES_PER_SECOND;
    desired.format = AUDIO_F32;
    desired.channels = 2;
    desired.samples = audio_settings->buffer_size;
    desired.callback = nullptr;

    SDL_AudioSpec obtained {};

    audio_settings->dev = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);

    if (audio_settings->dev == 0)
    {
        printf("SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        return;
    }

    SDL_PauseAudioDevice(audio_settings->dev, 0);
}

int main(int argc, char* argv[])
{
    AudioSettings audio_settings = { 
        .samples_per_second = SAMPLES_PER_SECOND,
        .dev = 0,
        .buffer_size = AUDIO_SAMPLES
    };
    SDL_Init( SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO );
    SDL_Window* window = SDL_CreateWindow("My SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 2000, 800,
                                          SDL_WINDOW_RESIZABLE);

	open_game_controllers();
    // open_audio_device(&audio_settings, AudioData);

    // Other way of opening audio (PS: uncomment if statement block in while loop too)
    other_audio(&audio_settings);

    if (window)
    {
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer)
        {
			bool running = true;

			// NOTE: We are currently assignming the windows dimensions (which hyprland decides)
			// to our backbuffer (buf containing our pixels). We don't necessarily have to!
			// (use can really use whatever you want, in order to size the pixel squares)
			WindowDimensions dimensions = get_window_dimensions(window);
            resize_texture(&GlobalBackbuffer, renderer, dimensions.width, dimensions.height);
			int xOffset = 0;
			int yOffset = 0;
			while (running)
            {
                if (SDL_GetQueuedAudioSize(audio_settings.dev) < sizeof(AudioData) * 4)
                {
                    float new_tone = BASE_TONE_FREQUENCY * powf(2.0f, (float)TargetToneInterval / 12.0f);
                    set_audio_tone(AudioData, new_tone, AUDIO_SAMPLES);
                    SDL_QueueAudio(audio_settings.dev, AudioData, sizeof(AudioData));
                }

                SDL_Event event;
                while (SDL_PollEvent(&event))
				{
					if (event_callback(&GlobalBackbuffer, &event))
					{
						running = false;
					}
				}
				for (int ctrl_idx{}; ctrl_idx < MAX_CONTROLLERS; ++ctrl_idx)
				{
					if (ControllerHandles[ctrl_idx] != 0 && SDL_GameControllerGetAttached(ControllerHandles[ctrl_idx]))
					{
						bool up         = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_DPAD_UP);
						bool down       = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
						bool left       = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
						bool right      = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
						bool start      = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_START);
						bool back       = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_BACK);
						bool l_shoulder = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
						bool r_shoulder = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
						bool a_button   = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_A);
						bool b_button   = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_B);
						bool x_button   = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_X);
						bool y_button   = SDL_GameControllerGetButton(ControllerHandles[ctrl_idx], SDL_CONTROLLER_BUTTON_Y);

						int16_t stick_x = SDL_GameControllerGetAxis(ControllerHandles[ctrl_idx], SDL_CONTROLLER_AXIS_LEFTX);
						int16_t stick_y = SDL_GameControllerGetAxis(ControllerHandles[ctrl_idx], SDL_CONTROLLER_AXIS_LEFTY);

						xOffset += stick_x >> 12;
						yOffset += stick_y >> 12;
						printf("stick_x: %d\t\t\tstick_y: %d\n", stick_x, stick_y);

						if (b_button)
						{
							if (RumbleHandles[ctrl_idx])
							{
								SDL_HapticRumblePlay(RumbleHandles[ctrl_idx], 0.5f, 100);
							}
						}
					}
					else
					{
						// No gamepads plugged in
					}
				}

				render_weird_gradient(&GlobalBackbuffer, xOffset, yOffset);
				render_weird_rectangleshape(&GlobalBackbuffer);
				display_buf_in_window(&GlobalBackbuffer, window, renderer);

				// ++xOffset;
				// yOffset -= 2;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_CloseAudio();
    SDL_Quit();

    return 0;
}
