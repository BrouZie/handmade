#include <SDL2/SDL.h>
#include <cstdlib>

#define internal static
#define global_variable static
#define local_persist static

// TODO: This is a global for now.
global_variable bool Running;

global_variable SDL_Texture* Texture;
global_variable void* Pixels;
global_variable int Texture_width;

internal void resize_texture(SDL_Renderer* renderer, int width, int height)
{
	if (Pixels)
	{
		free(Pixels);
	}
	if (Texture)
	{
		SDL_DestroyTexture(Texture);
	}

	Texture = SDL_CreateTexture(renderer,
							 	SDL_PIXELFORMAT_ARGB8888,
								SDL_TEXTUREACCESS_STREAMING,
								width,
								height);
	Texture_width = width;
	Pixels 		  = malloc(width * height * 4);
}

internal void update_window(SDL_Window* window, SDL_Renderer* renderer)
{
	SDL_UpdateTexture(Texture, nullptr, Pixels, Texture_width * 4);
	SDL_RenderCopy   (renderer, Texture, nullptr, nullptr);
	// SDL_RenderClear  (renderer);
   	SDL_RenderPresent(renderer);
}

void event_callback(SDL_Event* event)
{
    switch (event->type)
    {
		case SDL_QUIT:
		{
			Running = false;
		    printf("SDL_Quit\n");
		} break;

		case SDL_WINDOWEVENT:
		{
			switch(event->window.event)
			{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					SDL_Window*   window   { SDL_GetWindowFromID(event->window.windowID) };
					SDL_Renderer* renderer { SDL_GetRenderer(window) };

					resize_texture(renderer, event->window.data1, event->window.data2);
					printf("%d, %d\n", event->window.data1, event->window.data2);
				} break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				{
					printf("Keyboard focused\n");
				} break;

				// Equivalent to Casey's WM_PAINT case
				case SDL_WINDOWEVENT_EXPOSED:
				{
					// local_persist bool is_white = true;
					SDL_Window*   window   = SDL_GetWindowFromID(event->window.windowID);
					SDL_Renderer* renderer = SDL_GetRenderer(window);
					update_window(window, renderer);
				} break;
			}
		} break;
    }
    // return terminate_app;
}

int main(int argc, char* argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("My SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                          SDL_WINDOW_RESIZABLE);
    if (window)
    {
        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer)
        {
			Running = true;
			while (Running)
            {
                SDL_Event event;
                SDL_WaitEvent(&event);

                event_callback(&event); // Removed if
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
