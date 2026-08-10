#include "context.h"
#include "draw.h"
#include "draw_resrc.h"
#include "enemy.h"
#include "entity.h"
#include "log.h"
#include <assert.h>
#include <math.h>
#include <SDL3/SDL.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

int window_w = 0, window_h = 0;

arena omni_arena = {0};
arena tick_arenas[2] = {0};
size_t tick_arena_cur_idx = 0;
arena *tick_arena = NULL;

const bool *keyb_state = NULL;
bool *last_keyb_state = NULL;

u32 time_ms = 0;
u32 deltatime_ms = 0;
f32 time_s = 0;
f32 deltatime_s = 0;

u32 ticks_cnt = 0;
u32 tick_delta_ms = 20;
f32 tick_alpha = 0;
bool tick_flag = false;

u32 texture_new_help(const char *name) {
    char dir[128] = _ROOT_DIR "/asset/img/";
    strcat(dir, name);
    const char *ex = ".png";
    strcat(dir, ex);

    return texture_new(dir, renderer, SDL_SCALEMODE_NEAREST);
}

bool HasFRectIntersection(const SDL_FRect* a, const SDL_FRect* b) {
    if (!a || !b) return false;

    return (a->x < b->x + b->w) &&
           (a->x + a->w > b->x) &&
           (a->y < b->y + b->h) &&
           (a->y + a->h > b->y);
}

u32 player_entity = 0;
u32 player_sprite = 0;

vec2 player_input = {0};
vec2 player_atk_input = {0};
bool player_dash_input = false;

const f32 player_move_speed = 1;
const f32 player_atk_cd = 1;
const f32 player_atk_dur = .3;
const f32 player_dash_speed = 3;
const f32 player_dash_cd = 1;
const f32 player_dash_dur = .05;
const f32 player_atk_damage = 10;

vec2 player_move_dir = {0};
vec2 player_move_dir_nozero = {0};

bool player_atk_able = false;
bool player_atking = false;
bool player_just_atk = false;
char player_atk_dir = 0; // RLUD
f32 player_atk_timest = -1000;
u32 *player_atk_hitted_etts = 0;
size_t player_atk_hitted_etts_cap = 10;
size_t player_atk_hitted_etts_len = 0;

bool player_dash_able = false;
bool player_dashing = false;
bool player_just_dash = false;
vec2 player_dash_dir = {0};
f32 player_dash_timest = -1000;

void setup() {
    arena_init(&omni_arena, 100ull << 10 << 10); // 100mB

    arena_init_in_arena(&tick_arenas[0], &omni_arena, 10ull << 10 << 10); // 10mB
    arena_init_in_arena(&tick_arenas[1], &omni_arena, 10ull << 10 << 10); // 10mB

    { // input
    int numkeys;
    SDL_GetKeyboardState(&numkeys);
    last_keyb_state = arena_alloc(&omni_arena, numkeys, 1);
    }

    // Tex
    texture_init(128);
    texture_new_help("img_player");
    texture_new_help("img_enemy");

    // Sprite
    sprite_init(1024);
    SDL_FRect srect = {0, 0, 20, 20};

    // Draw
    draw_init(1024, 128);

    // Entity
    entity_init(1024);

    // Player
    player_sprite = sprite_new(1, (rect){0, 0, 20, 20});

    player_entity = entity_new(NULL);

    // Enemy
    slime_sprite = sprite_new(2, (rect){0, 0, 20, 20});

    enemy_init(1ull << 10 << 10); // 10 mB

    slime_init(128);

    slime s;
    entity s_ett;
    s_ett = (entity){ 0b1, (vec2){ 10, 10 }, (vec2){50, 50}, 36 };
    s = (slime){ entity_new(&s_ett), player_entity };
    slime_new(&s);
    s_ett = (entity){ 0b1, (vec2){ 100, 100 }, (vec2){50, 50}, 36 };
    s = (slime){ entity_new(&s_ett), player_entity };
    slime_new(&s);
    s_ett = (entity){ 0b1, (vec2){ 300, 500 }, (vec2){50, 50}, 10 };
    s = (slime){ entity_new(&s_ett), player_entity };
    slime_new(&s);
}

void input_update() {
    player_input = (vec2){0};
    if (keyb_state[SDL_SCANCODE_D]) player_input.x += 1;
    if (keyb_state[SDL_SCANCODE_A]) player_input.x -= 1;
    if (keyb_state[SDL_SCANCODE_W]) player_input.y += 1;
    if (keyb_state[SDL_SCANCODE_S]) player_input.y -= 1;
    vec2_normalize(&player_input);

    player_atk_input = (vec2){0};
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
    vec2 move_delta; vec2_scale(&move_delta, player_move_dir, player_move_speed * tick_delta_ms);

    if (player_just_atk) { }
    if (player_atking) {
        vec2 *player_pos = &entity_ptr(player_entity)->pos;
        vec2 atk_dir = { 1, 0 };
        if (player_atk_dir == 1) atk_dir = (vec2){ -1,  0 };
        if (player_atk_dir == 2) atk_dir = (vec2){  0,  1 };
        if (player_atk_dir == 3) atk_dir = (vec2){  0, -1 };
        vec2 atk_hitbox_pos = { player_pos->x + atk_dir.x * 100, player_pos->y + atk_dir.y * 100};
        vec2 atk_hitbox_size = { 100 + fabs(atk_dir.x) * 100, 100 + fabs(atk_dir.y) * 100 };
        SDL_FRect hitbox_rect = { atk_hitbox_pos.x - atk_hitbox_size.x / 2,
                                  atk_hitbox_pos.y - atk_hitbox_size.y / 2,
                                  atk_hitbox_size.x, atk_hitbox_size.y };

        // NOTE need to keep this run in every tick white player_atking
        void *new_hitted = arena_alloc(tick_arena, player_atk_hitted_etts_cap * sizeof(u32), alignof(u32));
        memcpy(new_hitted, player_atk_hitted_etts, player_atk_hitted_etts_len * sizeof(u32));
        player_atk_hitted_etts = new_hitted;

        for (size_t i = 0; i < entity_pool.len; ++i) {
            if (!pool_alive(&entity_pool, i)) continue;

            entity *entity = entity_ptr(i);
            if (!entity->tag) continue;

            SDL_FRect rect = { entity->pos.x - entity->bounds.x / 2,
                               entity->pos.y - entity->bounds.y / 2,
                               entity->bounds.x, entity->bounds.y };

            if (HasFRectIntersection(&hitbox_rect, &rect)) {
                size_t hitted = (size_t)-1;
                for (size_t j = 0; j < player_atk_hitted_etts_len; ++j) {
                    if (player_atk_hitted_etts[j] == (u32)i) {
                        hitted = j;
                        break;
                    }
                }

                if (hitted == (size_t)-1) {
                    player_atk_hitted_etts[player_atk_hitted_etts_len++] = i;
                    entity->health -= player_atk_damage;
                }

                if (player_atk_hitted_etts_len >= player_atk_hitted_etts_cap) break;
            }
        }
    } else { player_atk_hitted_etts_len = 0; } // FIXME delete

    if (player_dashing) {
        vec2_scale(&move_delta, player_dash_dir, player_dash_speed * tick_delta_ms);
    }

    vec2 *player_pos = &entity_ptr(player_entity)->pos;
    vec2_add(player_pos, *player_pos, move_delta);

    // =====================
    player_just_atk = false;
    }

    slime_update();
}

void frame_update() {

}

void render_update() {
    vec2 *player_pos = &entity_ptr(player_entity)->pos;
    draw_sprite_wpos(player_sprite, *player_pos, 0, (vec2){.5, .5}, (vec2){4, 4});
    slime_draw();

    if (player_atking) {
        vec2 atk_dir = { 1, 0 };
        if (player_atk_dir == 1) atk_dir = (vec2){ -1,  0 };
        if (player_atk_dir == 2) atk_dir = (vec2){  0,  1 };
        if (player_atk_dir == 3) atk_dir = (vec2){  0, -1 };
        vec2 atk_hitbox_pos = { player_pos->x + atk_dir.x * 100, player_pos->y + atk_dir.y * 100};
        vec2 atk_hitbox_size = { 100 + fabs(atk_dir.x) * 100, 100 + fabs(atk_dir.y) * 100 };

        draw_rect_wpos(atk_hitbox_pos, atk_hitbox_size, (vec2){.5, .5}, 255, 0, 0, 255);
    }
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }

    if (!SDL_CreateWindowAndRenderer("BurningFloor",
                                     800, 600, 0,
                                     &window, &renderer)) { return 1; }
    SDL_GetWindowSizeInPixels(window, &window_w, &window_h);

    setup();

    u32 last_time_ms = 0;
    u32 accml_time_ms = 0;
    while (true) {
        time_ms = SDL_GetTicks();
        deltatime_ms = time_ms - last_time_ms;
        last_time_ms = time_ms;

        time_s = (f32)time_ms / 1000.0f;
        deltatime_s = (f32)deltatime_ms / 1000.0f;

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

            tick_arena = &tick_arenas[tick_arena_cur_idx];
            tick_arena_cur_idx += 1;
            tick_arena_cur_idx %= 2;
            update();

            input_clean();
        }
        tick_alpha = (f32)accml_time_ms / (f32)tick_delta_ms;

        frame_update();

        // render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        render_update();

        draw_present();

        SDL_RenderPresent(renderer);

        // const u32 CAP_FPS = 60;
        // u32 _d = SDL_GetTicks() - time_ms;
        // u32 _fps = 1000ull / CAP_FPS;
        // SDL_Delay(_fps > _d ? _d : 0);
    }

END:
    arena_destroy(&omni_arena);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
