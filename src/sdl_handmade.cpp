#include <SDL2/SDL.h>

#define internal static
#define global_variable static
#define local_persist static

// TODO: This is a global for now.
global_variable bool Running;

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
				case SDL_WINDOWEVENT_RESIZED:
				{
					printf("%d, %d\n", event->window.data1, event->window.data2);

				} break;

				case SDL_WINDOWEVENT_FOCUS_GAINED:
				{
					printf("Keyboard focused\n");
				} break;

				case SDL_WINDOWEVENT_EXPOSED:
				{
					local_persist bool is_white = true;
					SDL_Window* window = SDL_GetWindowFromID(event->window.windowID);
					SDL_Renderer* renderer = SDL_GetRenderer(window);

					if (is_white)
					{
						SDL_SetRenderDrawColor(renderer, 255, 255, 255, 1);
						is_white = false;
					}
					else
					{
						SDL_SetRenderDrawColor(renderer, 0, 0, 0, 1);
						is_white = true;
					}
                    SDL_RenderClear(renderer);
                    SDL_RenderPresent(renderer);
				} break;
			}
		} break;
    }
    // return terminate_app;
}

internal void re_render(SDL_Renderer* renderer, int width, int height)
{
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
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

				int width, height;
				SDL_GetWindowSize(window, &width, &height);

				// re_render(renderer, width, height);
				// printf("%d, %d\n", width, height);
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
