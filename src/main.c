#include <SDL3/SDL.h>
#include <float.h>

#include "common.h"
#include "enemy/enemy.h"
#include "player/player.h"
#include "global.h"
#include "log/log.h"
#include "input/input.h"

vec2 _vec2_zero = {0, 0};
vec2 _vec2_one = {1, 1};
vec2 _vec2_half = {.5, .5};

int window_width = 1280;
int window_height = 720;
SDL_Window *sdl_window = NULL;
SDL_Renderer *sdl_renderer = NULL;

int tps = 50;
int pixel_size = 4;

float cur_frame_time = 0;
float cur_logic_time = 0;
float logic_update_alpha = 0;

spn_Context cfg_context = {0};

EttMng ett_mng = {0};

ImgMng img_mng = {0};
SprMng spr_mng = {0};
DrwrMng drwr_mng = {0};

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

    const size_t ETT_CAP = 32768;
    ett_mng_init(&ett_mng, ETT_CAP);

    const size_t IMAGE_CAP = 1024;
    const size_t SPRITE_CAP = 16384;
    const size_t DRAWER_CAP = 32768;
    img_mng_init(&img_mng, IMAGE_CAP);
    spr_mng_init(&spr_mng, SPRITE_CAP, &img_mng);
    drwr_mng_init(&drwr_mng, DRAWER_CAP, &spr_mng, 4, 1);

    const size_t ENEMY_CAP = 1024;
    enemy_mng_init(&enemy_mng, ENEMY_CAP);

    player_init();

    Key _e = enemy_new(&enemy_mng);
    vec2 _ep = {100, 100};
    enemy_slime_init(&enemy_mng, _e, _ep);
    enemy_get(&enemy_mng, _e)->target = player.ett;

    bool running = 1;
    uint64_t last_time_ns = SDL_GetTicksNS();
    float logic_update_accumulator = 0.0;
    while (running) {
        uint64_t cur_time_ns = SDL_GetTicksNS();
        float dt = (cur_time_ns - last_time_ns) / 1e9;
        last_time_ns = cur_time_ns;

        if (dt > 1) dt = 1; // still need 1 fps

        cur_frame_time += dt;
        logic_update_accumulator += dt;

        input_update(&running);

        const float logic_update_dt = (float)1.0 / tps;
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

    drwr_mng_destroy(&drwr_mng);
    spr_mng_destroy(&spr_mng);
    img_mng_destroy(&img_mng);

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void logic_update() {
    enemy_mng_update(&enemy_mng);

    player_logic_update();
}

void frame_update() {
    player_frame_update();
}

void render_update() {
    drwr_mng_update(&drwr_mng);

    SDL_SetRenderDrawColor(sdl_renderer, 155, 155, 155, 255);
    SDL_RenderClear(sdl_renderer);

    drwr_mng_draw(&drwr_mng, sdl_renderer, sdl_window);

    SDL_RenderPresent(sdl_renderer);
}
