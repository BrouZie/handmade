#include <SDL2/SDL.h>
#include <cstdlib>
#include <sys/mman.h>
#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

// TODO: clean this global mess
global_variable SDL_Texture* Texture;
global_variable void* BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel=4;

internal void
render_weird_gradient(int BlueOffset, int GreenOffset)
{    
    int width = BitmapWidth;
    int height = BitmapHeight;

    int pitch = width*BytesPerPixel;
    uint8_t *row = (uint8_t *)BitmapMemory;    
    for(int y = 0; y < BitmapHeight; ++y)
	{
        uint32_t *pixel = (uint32_t *)row;
        for(int x {}; x < BitmapWidth; ++x)
		{
            uint8_t Blue = (x + BlueOffset);
            uint8_t Green = (y + GreenOffset);
            
            *pixel++ = ((Green << 8) | Blue);
        }

        row += pitch;
    }
}

internal void resize_texture(SDL_Renderer* renderer, int width, int height)
{
    if (BitmapMemory)
    {
        munmap(BitmapMemory,
               BitmapWidth * BitmapHeight * BytesPerPixel);
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
    BitmapWidth  = width;
    BitmapHeight = height;
    BytesPerPixel = 4;

    BitmapMemory = mmap(nullptr,
						BitmapWidth * BitmapHeight * BytesPerPixel,
						PROT_READ | PROT_WRITE,
						MAP_ANONYMOUS | MAP_PRIVATE,
						-1,
						0);

	// TODO: Clear bitmap to black
}

internal void update_window(SDL_Window* window, SDL_Renderer* renderer)
{
	SDL_UpdateTexture(Texture, nullptr, BitmapMemory, BitmapWidth * 4);
	SDL_RenderCopy   (renderer, Texture, nullptr, nullptr);
	// SDL_RenderClear  (renderer);
   	SDL_RenderPresent(renderer);
}

bool event_callback(SDL_Event* event)
{
	bool terminate_app = false;
    switch (event->type)
    {
		case SDL_QUIT:
		{
            terminate_app = true;
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
    return terminate_app;
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
			bool Running = true;

			int xOffset = 0;
			int yOffset = 0;

			// NOTE: What the fuck is this needed for?? Is it needed at all?
            // int Width, Height;
            // SDL_GetWindowSize(window, &Width, &Height);
            // resize_texture(renderer, Width, Height);
			while (Running)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
				{
					if (event_callback(&event))
					{
						Running = false;
					}
				}
				render_weird_gradient(xOffset, yOffset);
				update_window(window, renderer);

				++xOffset;
				yOffset += 2;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
