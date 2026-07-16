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

// ImageSys image_sys = {0};

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

    // img_sys_init(&image_sys, IMAGE_CAP, DRAWER_CAP);

    player_init();

    enemy_defs_init();

    const size_t ENEMY_CAP = 128;
    enemy_mng_init(&enemy_mng, ENEMY_CAP);

    // FIXME
    enemy_make(&enemy_mng, ENEMY_MELEE);

    enemy_melee_init(&enemy_mng, 1);

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

    // img_sys_destroy(&image_sys);

    player_destroy();

    enemy_mng_destroy(&enemy_mng);

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void logic_update() {
    player_logic_update();

    enemy_melee_update_behavior(&enemy_mng, 1);
}

void frame_update() {
    player_frame_update();
}

void render_update() {
    player_render_update();
    enemy_melee_update_render(&enemy_mng, 1);

    SDL_SetRenderDrawColor(sdl_renderer, 155, 155, 155, 255);
    SDL_RenderClear(sdl_renderer);

    player_draw();
    enemy_melee_draw(&enemy_mng, 1);

    SDL_RenderPresent(sdl_renderer);
}
