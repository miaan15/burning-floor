#include "context.h"
#include "log.h"
#include <SDL3/SDL.h>

Arena global_ar = {0};

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

    SDL_Surface *surf = SDL_LoadPNG(_ROOT_DIR "/asset/img/img_player.png");
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surf);

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                goto END;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_FRect srect = {0, 0, 20, 20};
        SDL_FRect drect = {100, 100, 200, 200};
        SDL_RenderTexture(renderer, tex, &srect, &drect);

        SDL_RenderPresent(renderer);
    }

END:
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
