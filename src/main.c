#include <stdio.h>
#include <SDL3/SDL.h>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;
    if (!SDL_CreateWindowAndRenderer("BurningFloor",
                                     800, 600, 0,
                                     &window, &renderer)) {
        return 1;
    }

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                goto END;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }

END:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
