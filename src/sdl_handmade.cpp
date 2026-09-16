#include <SDL2/SDL.h>
#include <sys/mman.h>
#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

// TODO: clean this global mess
//
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
global_variable bool GlobalRunning;

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
			GlobalRunning = true;

			// NOTE: We are currently assignming the windows dimensions (which hyprland decides)
			// to our backbuffer (buf containing our pixels). We don't necessarily have to!
			// (use can really use whatever you want, in order to size the pixel squares)
			WindowDimensions dimensions = get_window_dimensions(window);
            resize_texture(&GlobalBackbuffer, renderer, dimensions.width, dimensions.height);
			int xOffset = 0;
			int yOffset = 0;
			while (GlobalRunning)
            {
                SDL_Event event;
                while (SDL_PollEvent(&event))
				{
					if (event_callback(&GlobalBackbuffer, &event))
					{
						GlobalRunning = false;
					}
				}
				render_weird_gradient(&GlobalBackbuffer, xOffset, yOffset);
				display_buf_in_window(GlobalBackbuffer, window, renderer);

				++xOffset;
				yOffset -= 2;
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
