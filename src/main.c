#include "context.h"
#include "log.h"
#include <SDL3/SDL.h>
#include <stdalign.h>
#include <string.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

Arena global_ar = {0};

const bool *keyb_state = NULL;
bool *last_keyb_state = NULL;

SDL_Texture **texs = NULL;
size_t texs_len = 0;

void register_tex(const char *name) {
    char dir[128] = _ROOT_DIR "/asset/img/";
    strcat(dir, name);
    const char *ex = ".png";
    strcat(dir, ex);

    SDL_Surface *surf = SDL_LoadPNG(dir);
    if (surf == NULL) log_err("create surface failed: name: %s: %s", name, SDL_GetError());
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex == NULL) log_err("create texture failed: name: %s: %s", name, SDL_GetError());

    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);

    texs[texs_len++] = tex;

    SDL_DestroySurface(surf);
}

enum {
    TEX_PLAYER = 0,
    TEX_ENEMY,
    TEX_VFX,
};

Vec2 player_input = {0};
Vec2 player_pos = {0};

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }

    if (!SDL_CreateWindowAndRenderer("BurningFloor",
                                     800, 600, 0,
                                     &window, &renderer)) { return 1; }

    arena_init(&global_ar, 100u << 10 << 10); // 100mB

    { // input
    int numkeys;
    SDL_GetKeyboardState(&numkeys);
    last_keyb_state = arena_alloc(&global_ar, numkeys, 1);
    }

    { // tex
    const size_t MAX_TEX = 16;
    texs = arena_alloc(&global_ar, MAX_TEX * sizeof(SDL_Texture *), alignof(SDL_Texture *));
    register_tex("img_player");
    register_tex("img_enemy");
    register_tex("img_vfx");
    }

    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                goto END;
            }
        }

        { // input
        int numkeys;
        keyb_state = SDL_GetKeyboardState(&numkeys);

        if (keyb_state[SDL_SCANCODE_W] && !last_keyb_state[SDL_SCANCODE_W]) log_info("XD");

        player_input = (Vec2){0};
        if (keyb_state[SDL_SCANCODE_D]) player_input.x += 1;
        if (keyb_state[SDL_SCANCODE_A]) player_input.x -= 1;
        if (keyb_state[SDL_SCANCODE_W]) player_input.y += 1;
        if (keyb_state[SDL_SCANCODE_S]) player_input.y -= 1;
        vec2_normalize(&player_input);

        memcpy(last_keyb_state, keyb_state, numkeys);
        }

        {
        vec2_add(&player_pos, player_pos, player_input);
        }

        // render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_FRect srect = {0, 0, 20, 20};
        SDL_FRect drect = {100 + player_pos.x, 100 - player_pos.y, 200, 200};
        SDL_RenderTexture(renderer, texs[TEX_PLAYER], &srect, &drect);

        SDL_RenderPresent(renderer);
    }

END:
    arena_destroy(&global_ar);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
