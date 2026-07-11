#include <SDL3/SDL.h>
#include <float.h>
#include <math.h>

#include "macro.h"
#include "common.h"
#include "global.h"
#include "log/log.h"
#include "input/input.h"
#include "game/player.h"
#include "game/enemy.h"

int window_width = 1280;
int window_height = 720;
SDL_Window *sdl_window = NULL;
SDL_Renderer *sdl_renderer = NULL;

int tps = 50;
int pixel_size = 4;

double cur_frame_time = 0;
double cur_logic_time = 0;
double logic_update_alpha = 0;

spn_Context cfg_context = {0};

ImageSys image_sys = {0};

void logic_update();
void frame_update();
void render_update();

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_err("%s", SDL_GetError());
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("BurningFloor",
                                     window_width, window_height,
                                     0,
                                     &sdl_window, &sdl_renderer)) {
        log_critical("%s", SDL_GetError());
        return 1;
    }

    { // Parse configs
    const char *cfg_path = ASSET_PATH "/config.txt";
    FILE *cfg_file = fopen(cfg_path, "r");
    if (cfg_file == NULL) {
        log_critical("Error open: %s", cfg_path);
        return 1;
    }

    if (fseek(cfg_file, 0, SEEK_END) != 0) {
        log_critical("Error seek file: %s", cfg_path);
        fclose(cfg_file);
        return 1;
    }

    int cfg_size = ftell(cfg_file);
    if (cfg_size < 0) {
        log_critical("Error ftell: %s", cfg_path);
        fclose(cfg_file);
        return 1;
    }

    rewind(cfg_file);

    char *cfg_buffer = malloc(cfg_size + 1);
    fread(cfg_buffer, 1, cfg_size, cfg_file);
    cfg_buffer[cfg_size] = '\0';

    fclose(cfg_file);

    spn_parse(&cfg_context, cfg_buffer, cfg_size);

    free(cfg_buffer);
    }

    input_init();

    const size_t IMAGE_CAP = 128;
    const size_t DRAWER_CAP = 1024;

    img_sys_init(&image_sys, IMAGE_CAP, DRAWER_CAP);

    { // Player
    spn_Mark cfgm_pl = spn_find(spn_root(&cfg_context), "asset/player");

    { //
    spn_move(&cfgm_pl, "image");
    const char *image_path = spn_get_str(&cfgm_pl, 0);
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", ASSET_PATH, image_path);
    player_data.image =
        img_load_tex(&image_sys, full_path, SDL_SCALEMODE_NEAREST);
    player_data.drawer = img_make_drawer(&image_sys, player_data.image);
    }

    cfgm_pl = spn_find(spn_root(&cfg_context), "game/player");

    spn_move(&cfgm_pl, "move_speed");
    player_def.move_speed = spn_get_float(&cfgm_pl, 0);

    spn_move_sibling(&cfgm_pl, "move_ani_delta");
    player_def.move_ani_delta = spn_get_float(&cfgm_pl, 0);

    spn_move_sibling(&cfgm_pl, "attack_duration");
    player_def.attack_duration = spn_get_float(&cfgm_pl, 0);

    spn_move_sibling(&cfgm_pl, "attack_cooldown");
    player_def.attack_cooldown = spn_get_float(&cfgm_pl, 0);

    spn_move_sibling(&cfgm_pl, "attack_hold_ani_delta");
    player_def.attack_hold_ani_delta = spn_get_float(&cfgm_pl, 0);

    spn_move_sibling(&cfgm_pl, "attack_act_ani_delta");
    player_def.attack_act_ani_delta = spn_get_float(&cfgm_pl, 0);
    }

    bool running = 1;
    uint64_t last_time_ns = SDL_GetTicksNS();
    double logic_update_accumulator = 0.0;
    while (running) {
        uint64_t cur_time_ns = SDL_GetTicksNS();
        double dt = (double)(cur_time_ns - last_time_ns) / 1000000000.0;
        last_time_ns = cur_time_ns;

        if (dt > 1) dt = 1; // still need 1 fps

        cur_frame_time += dt;
        logic_update_accumulator += dt;

        input_update(&running);

        const double logic_update_dt = (double)1.0 / tps;
        while (logic_update_accumulator >= logic_update_dt) {
            cur_logic_time += logic_update_dt;

            logic_update();
            logic_update_accumulator -= logic_update_dt;
        }

        logic_update_alpha =
            logic_update_accumulator / logic_update_dt;

        frame_update();

        render_update();
    }

    img_sys_destroy(&image_sys);

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void logic_update() {
    // Get from move input
    const float EPSILON = 0.0001;
    player_data.move_dir = player_data.move_input;
    float move_input_len =
        sqrtf((player_data.move_dir.x * player_data.move_dir.x) +
              (player_data.move_dir.y * player_data.move_dir.y));
    if (move_input_len > EPSILON) {
        player_data.move_dir.x /= move_input_len;
        player_data.move_dir.y /= move_input_len;
    } else {
        player_data.move_dir.x = 0;
        player_data.move_dir.y = 0;
    }

    // Get from attack input
    player_data.attack_trigger = 0;
    if (player_data.can_attack) {
        if (player_data.attack_input) player_data.attack_trigger = 1;
    }
    player_data.attack_input = 0;

    // Attack
    if (player_data.attack_trigger) {
        player_data.can_attack = 0;
        player_data.attacking = 1;

        player_data.attack_end_time = cur_logic_time + player_def.attack_duration;
        player_data.attack_off_cd_time = cur_logic_time + player_def.attack_cooldown;

        player_data.attack_dir = player_data.facing_dir;
    }

    if (cur_logic_time > player_data.attack_end_time) {
        player_data.attack_end_time = INFINITY;
        player_data.attacking = 0;
    }

    if (player_data.attacking) {
        // TODO attack logic
        player_data.move_dir.x = player_data.attack_dir.x;
        player_data.move_dir.y = player_data.attack_dir.y;
    }

    if (cur_logic_time > player_data.attack_off_cd_time) {
        player_data.can_attack = 1;
        player_data.attack_end_time = INFINITY;
    }

    // Move
    if (!player_data.attacking) {
        player_data.pos.x += player_data.move_dir.x * player_def.move_speed;
        player_data.pos.y += player_data.move_dir.y * player_def.move_speed;
    }
}

void frame_update() {
    player_data.move_input = (Vec2){0};
    if (is_key_on(SCANCODE_W) || is_key_on(SCANCODE_UP))
        player_data.move_input.y += 1;
    if (is_key_on(SCANCODE_A) || is_key_on(SCANCODE_LEFT))
        player_data.move_input.x -= 1;
    if (is_key_on(SCANCODE_S) || is_key_on(SCANCODE_DOWN))
        player_data.move_input.y -= 1;
    if (is_key_on(SCANCODE_D) || is_key_on(SCANCODE_RIGHT))
        player_data.move_input.x += 1;

    if (is_key_down(SCANCODE_Z) || is_key_down(SCANCODE_J)) {
        player_data.attack_input = 1;
    }

    const float EPSILON = 0.0001;
    player_data.move_dir = player_data.move_input;
    float move_input_len =
        sqrtf((player_data.move_dir.x * player_data.move_dir.x) +
              (player_data.move_dir.y * player_data.move_dir.y));
    if (move_input_len > EPSILON) {
        if (fabsf(player_data.move_input.x) > EPSILON) {
            player_data.facing_dir.x = copysignf(1, player_data.move_input.x);
            player_data.facing_dir.y = 0;
        } else {
            player_data.facing_dir.x = 0;
            player_data.facing_dir.y = copysignf(1, player_data.move_input.y);
        }
    }

    // Move Animation
    if (player_data.facing_dir.y == 1)
        player_data.drawer_srect_pos = (Vec2){0, 20};
    else if (player_data.facing_dir.y == -1)
        player_data.drawer_srect_pos = (Vec2){0,  0};
    else if (player_data.facing_dir.x == 1)
        player_data.drawer_srect_pos = (Vec2){0, 60};
    else
        player_data.drawer_srect_pos = (Vec2){0, 40};

    if (move_input_len > EPSILON) {
        float delta = (float)cur_frame_time - player_data.last_move_frame_time;
        if (delta > player_def.move_ani_delta) {
            int n = (int)(delta / player_def.move_ani_delta);
            const int MAX_FRAME = 4;
            player_data.cur_move_frame = (player_data.cur_move_frame + n) % MAX_FRAME;

            player_data.last_move_frame_time = (float)cur_frame_time + n * delta;
        }

        player_data.drawer_srect_pos.x = 0 + player_data.cur_move_frame * 20;
    } else {
        player_data.cur_move_frame = 0;
        player_data.last_move_frame_time = cur_frame_time;
    }

    // Attack Animation
    if (player_data.attacking) {
        if (player_data.attack_dir.y == 1)
            player_data.drawer_srect_pos = (Vec2){80, 20};
        else if (player_data.attack_dir.y == -1)
            player_data.drawer_srect_pos = (Vec2){80,  0};
        else if (player_data.attack_dir.x == 1)
            player_data.drawer_srect_pos = (Vec2){80, 60};
        else
            player_data.drawer_srect_pos = (Vec2){80, 40};

        const int MAX_FRAME = 4;
        if (player_data.cur_attack_frame < MAX_FRAME) {
            float delta = (float)cur_frame_time - player_data.last_attack_frame_time;
            // FIXME logic error btw
            float cd = player_data.cur_attack_frame < 1 ? player_def.attack_hold_ani_delta
                                                        : player_def.attack_act_ani_delta;
            if (delta > cd) {
                int n = (int)(delta / cd);
                player_data.cur_attack_frame += n;
                if (player_data.cur_attack_frame > MAX_FRAME)
                    player_data.cur_attack_frame = MAX_FRAME;

                player_data.last_attack_frame_time = (float)cur_frame_time + n * delta;
            }
        }

        player_data.drawer_srect_pos.x = 80 + player_data.cur_attack_frame * 20;
    } else {
        player_data.cur_attack_frame = 0;
        player_data.last_attack_frame_time = cur_frame_time;
    }
}

void render_update() {
    img_get_drawer_ptr(&image_sys, player_data.drawer)->srect =
        (Rect){player_data.drawer_srect_pos.x, player_data.drawer_srect_pos.y,
               20, 20};
    img_feed_drawer_world(&image_sys, player_data.drawer, player_data.pos, 0, 1);

    SDL_SetRenderDrawColor(sdl_renderer, 155, 155, 155, 255);
    SDL_RenderClear(sdl_renderer);

    img_draw(&image_sys, player_data.drawer);

    SDL_RenderPresent(sdl_renderer);
}
