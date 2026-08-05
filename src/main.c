#include "context.h"
#include "log.h"
#include "draw.h"
#include <SDL3/SDL.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int window_w = 0, window_h = 0;

Arena global_ar = {0};

const bool *keyb_state = NULL;
bool *last_keyb_state = NULL;

uint64_t time_ms = 0;
uint64_t deltatime_ms = 0;
float time_s = 0;
float deltatime_s = 0;

uint64_t ticks_cnt = 0;
uint64_t tick_delta_ms = 20;
float tick_alpha = 0;
bool tick_flag = false;

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
    SDL_GetWindowSizeInPixels(window, &window_w, &window_h);

    arena_init(&global_ar, 100ull << 10); // 100mB

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

    draw_init(&draw_sys, 1024, 1024, 2, 2);

    SDL_FRect srect = {0, 0, 20, 20};
    DrawerResult res =
        draw_new_drawer(&draw_sys, draw_new_sprite(&draw_sys, texs[0], &srect), NULL);
    res.hook->active = true;
    res.hook->pos = &player_pos;

    uint64_t last_time_ms = 0;
    uint64_t accml_time_ms = 0;
    while (true) {
        time_ms = SDL_GetTicks();
        deltatime_ms = time_ms - last_time_ms;
        last_time_ms = time_ms;

        time_s = (float)time_ms / 1000.0f;
        deltatime_s = (float)deltatime_ms / 1000.0f;

        accml_time_ms += deltatime_ms;

        tick_flag = false;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                goto END;
            }
        }
        SDL_GetWindowSizeInPixels(window, &window_w, &window_h);

        { // input
        int numkeys;
        keyb_state = SDL_GetKeyboardState(&numkeys);

        player_input = (Vec2){0};
        if (keyb_state[SDL_SCANCODE_D]) player_input.x += 1;
        if (keyb_state[SDL_SCANCODE_A]) player_input.x -= 1;
        if (keyb_state[SDL_SCANCODE_W]) player_input.y += 1;
        if (keyb_state[SDL_SCANCODE_S]) player_input.y -= 1;
        vec2_normalize(&player_input);

        memcpy(last_keyb_state, keyb_state, numkeys);
        }

        while (accml_time_ms >= tick_delta_ms) {
            accml_time_ms -= tick_delta_ms;
            tick_flag = true;

            Vec2 move_delta = {0}; vec2_scale(&move_delta, player_input, tick_delta_ms);
            vec2_add(&player_pos, player_pos, move_delta);
        }
        tick_alpha = (float)accml_time_ms / (float)tick_delta_ms;

        // render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_update(&draw_sys);
        draw(&draw_sys);

        SDL_RenderPresent(renderer);

        // const uint64_t CAP_FPS = 60;
        // uint64_t _d = SDL_GetTicks() - time_ms;
        // uint64_t _fps = 1000ull / CAP_FPS;
        // SDL_Delay(_fps > _d ? _d : 0);
    }

END:
    arena_destroy(&global_ar);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
