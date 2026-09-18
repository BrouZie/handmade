#include <SDL2/SDL.h>
#include <SDL2/SDL_gamecontroller.h>
#include <sys/mman.h>
#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

struct OffscreenBuffer
{
	// NOTE: Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
	SDL_Texture* texture;
	void* memory;
	int width;
	int height;
	int pitch;
};

global_variable OffscreenBuffer GlobalBackbuffer;

#define MAX_CONTROLLERS 4

global_variable SDL_GameController* ControllerHandles[MAX_CONTROLLERS];
global_variable SDL_Haptic*         RumbleHandles[MAX_CONTROLLERS];

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

internal void display_buf_in_window(OffscreenBuffer buffer, SDL_Window* window, SDL_Renderer* renderer)
{
	// TODO: Aspect ratio correction
	SDL_UpdateTexture(buffer.texture, nullptr, buffer.memory, buffer.pitch);
	SDL_RenderCopy   (renderer, buffer.texture, nullptr, nullptr);
   	SDL_RenderPresent(renderer);
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

		case SDL_KEYDOWN:
		case SDL_KEYUP:
        {
            SDL_Keycode key_code = event->key.keysym.sym;
            bool is_down = (event->key.state == SDL_PRESSED);
            bool was_down = false;
            if (event->key.state == SDL_RELEASED)
            {
                was_down = true;
            }
            else if (event->key.repeat != 0)
            {
                was_down = true;
            }
            
            // NOTE: In the windows version, we used "if (IsDown != WasDown)"
            // to detect key repeats. SDL has the 'repeat' value, though,
            // which we'll use.
            if (event->key.repeat == 0)
            {
                if(key_code == SDLK_w)
                {
                }
                else if(key_code == SDLK_a)
                {
                }
                else if(key_code == SDLK_s)
                {
                }
                else if(key_code == SDLK_d)
                {
                }
                else if(key_code == SDLK_q)
                {
                }
                else if(key_code == SDLK_e)
                {
                }
                else if(key_code == SDLK_UP)
                {
                }
                else if(key_code == SDLK_LEFT)
                {
                }
                else if(key_code == SDLK_DOWN)
                {
                }
                else if(key_code == SDLK_RIGHT)
                {
                }
                else if(key_code == SDLK_ESCAPE)
                {
                    printf("ESCAPE: ");
                    if(is_down)
                    {
                        printf("IsDown ");
                    }
                    if(was_down)
                    {
                        printf("WasDown");
                    }
                    printf("\n");
                }
                else if(key_code == SDLK_SPACE)
                {
                }
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
					display_buf_in_window(*buffer, window, renderer);
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
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
    SDL_Window* window = SDL_CreateWindow("My SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                          SDL_WINDOW_RESIZABLE);
	open_game_controllers();

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

						if (up)
						{
							yOffset -= 5;
						}
						if (down)
						{
							yOffset += 5;
						}
						if (left)
						{
							xOffset -= 5;
						}
						if (right)
						{
							xOffset += 5;
						}

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
				display_buf_in_window(GlobalBackbuffer, window, renderer);

				// ++xOffset;
				// yOffset -= 2;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
