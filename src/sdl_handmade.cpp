#include <SDL2/SDL.h>

bool HandleEvent(SDL_Event* event)
{
    bool terminate_app = false;
    switch (event->type)
    {
        case SDL_QUIT:
            printf("SDL_Quit\n");
            terminate_app = true;
            break;
        case SDL_WINDOWEVENT:
            switch (event->window.event)
            {
                case SDL_WINDOWEVENT_RESIZED:      printf("%d, %d\n", event->window.data1, event->window.data2); break;
                case SDL_WINDOWEVENT_FOCUS_GAINED: printf("Keyboard focus\n"); break;
                case SDL_WINDOWEVENT_EXPOSED:      {
                    static bool is_white   = true;
                    SDL_Window* window     = SDL_GetWindowFromID(event->window.windowID);
                    SDL_Renderer* renderer = SDL_GetRenderer(window);

                    if (is_white)
                    {
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        is_white = false;
                    }
                    else
                    {
                        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                        is_white = true;
                    }
                    SDL_RenderClear(renderer);
                    SDL_RenderPresent(renderer);
                    break;
                }
            }
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
            for (;;)
            {
                SDL_Event event;
                SDL_WaitEvent(&event);

                if (HandleEvent(&event))
                {
                    break; // why though?
                }
            }
        }
    }
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
