#include "context.h"
#include "draw.h"
#include "draw_resource.h"
#include "log.h"
#include <math.h>
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

u32 texture_new_help(const char *name) {
    char dir[128] = _ROOT_DIR "/asset/img/";
    strcat(dir, name);
    const char *ex = ".png";
    strcat(dir, ex);

    return texture_new(dir, renderer, SDL_SCALEMODE_NEAREST);
}

Vec2 player_pos = {0};
u32 player_sprite = 0;

Vec2 player_input = {0};
Vec2 player_atk_input = {0};
bool player_dash_input = false;

const float player_move_speed = 1;
const float player_atk_cd = 1;
const float player_atk_dur = .3;
const float player_dash_speed = 3;
const float player_dash_cd = 1;
const float player_dash_dur = .05;

Vec2 player_move_dir = {0};
Vec2 player_move_dir_nozero = {0};

bool player_atk_able = false;
bool player_atking = false;
bool player_just_atk = false;
char player_atk_dir = 0; // RLUD
float player_atk_timest = -1000;

bool player_dash_able = false;
bool player_dashing = false;
bool player_just_dash = false;
Vec2 player_dash_dir = {0};
float player_dash_timest = -1000;

void setup() {
    arena_init(&global_ar, 100ull << 10 << 10); // 100mB

    { // input
    int numkeys;
    SDL_GetKeyboardState(&numkeys);
    last_keyb_state = arena_alloc(&global_ar, numkeys, 1);
    }

    // Tex
    texture_init(128);
    u32 tex = texture_new_help("img_player");

    // Sprite
    sprite_init(1024);
    SDL_FRect srect = {0, 0, 20, 20};
    player_sprite = sprite_new(tex, &srect);

    // Draw
    draw_init(1024);
}

void input_update() {
    player_input = (Vec2){0};
    if (keyb_state[SDL_SCANCODE_D]) player_input.x += 1;
    if (keyb_state[SDL_SCANCODE_A]) player_input.x -= 1;
    if (keyb_state[SDL_SCANCODE_W]) player_input.y += 1;
    if (keyb_state[SDL_SCANCODE_S]) player_input.y -= 1;
    vec2_normalize(&player_input);

    player_atk_input = (Vec2){0};
    if (keyb_state[SDL_SCANCODE_RIGHT]) player_atk_input.x += 1;
    if (keyb_state[SDL_SCANCODE_LEFT]) player_atk_input.x -= 1;
    if (keyb_state[SDL_SCANCODE_UP]) player_atk_input.y += 1;
    if (keyb_state[SDL_SCANCODE_DOWN]) player_atk_input.y -= 1;
    vec2_normalize(&player_atk_input);

    if (keyb_state[SDL_SCANCODE_SPACE] && !last_keyb_state[SDL_SCANCODE_SPACE]) {
        player_dash_input = true;
    }
}

void input_clean() {
    player_dash_input = false;
}

void update() {
    { // Player
    player_move_dir = player_input;
    if (player_move_dir.x != 0 || player_move_dir.y != 0)
        player_move_dir_nozero = player_move_dir;

    player_atk_able = time_s - player_atk_timest > player_atk_cd;
    if (player_atk_able) {
        if (player_atk_input.x != 0 || player_atk_input.y != 0) {
            player_just_atk = true;
            player_atking = true;

            player_atk_timest = time_s;

            if (fabs(player_atk_input.x) > 0)
                player_atk_dir = player_atk_input.x > 0 ? 0 : 1;
            else
                player_atk_dir = player_atk_input.y > 0 ? 2 : 3;
        }
    }

    if (time_s - player_atk_timest > player_atk_dur) player_atking = false;

    player_dash_able = time_s - player_dash_timest > player_dash_cd;
    if (player_dash_able) {
        if (player_dash_input) {
            player_just_dash = true;
            player_dashing = true;

            player_dash_timest = time_s;

            player_dash_dir = player_move_dir_nozero;
        }
    }

    if (time_s - player_dash_timest > player_dash_dur) player_dashing = false;

    // =====================
    Vec2 move_delta; vec2_scale(&move_delta, player_move_dir, player_move_speed * tick_delta_ms);

    if (player_just_atk) { }
    if (player_atking) { }

    if (player_dashing) {
        vec2_scale(&move_delta, player_dash_dir, player_dash_speed * tick_delta_ms);
    }

    vec2_add(&player_pos, player_pos, move_delta);

    // =====================
    player_just_atk = false;
    }
}

void frame_update() {

}

void render_update() {
    draw_sprite_wpos(player_sprite, player_pos, 0, (Vec2){.5, .5}, (Vec2){4, 4});
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }

    if (!SDL_CreateWindowAndRenderer("BurningFloor",
                                     800, 600, 0,
                                     &window, &renderer)) { return 1; }
    SDL_GetWindowSizeInPixels(window, &window_w, &window_h);

    setup();

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

        input_update();

        memcpy(last_keyb_state, keyb_state, numkeys);
        }

        while (accml_time_ms >= tick_delta_ms) {
            accml_time_ms -= tick_delta_ms;
            tick_flag = true;

            update();

            input_clean();
        }
        tick_alpha = (float)accml_time_ms / (float)tick_delta_ms;

        frame_update();

        // render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_update();
        draw();

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
