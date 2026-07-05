#include <SDL3/SDL.h>
#include <math.h>

#include "macro.h"
#include "common.h"
#include "global.h"
#include "log/log.h"
#include "input/input.h"
#include "game/player.h"

int window_width = 1280;
int window_height = 720;
SDL_Window *sdl_window = NULL;
SDL_Renderer *sdl_renderer = NULL;

int tps = 50;
int pixel_size = 4;

int cur_time = 0;
int logic_update_alpha = 0;

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
    }

    bool running = 1;
    uint64_t last_time_ns = SDL_GetTicksNS();
    double logic_update_accumulator = 0.0;
    while (running) {
        uint64_t cur_time_ns = SDL_GetTicksNS();
        double dt = (double)(cur_time_ns - last_time_ns) / 1000000000.0;
        last_time_ns = cur_time_ns;

        if (dt > 1) dt = 1; // still need 1 fps

        cur_time += dt;
        logic_update_accumulator += dt;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        input_pump();

        const double logic_update_dt = (double)1.0 / tps;
        while (logic_update_accumulator >= logic_update_dt) {
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
    player_data.pos.x += player_data.move_dir.x * player_def.move_speed;
    player_data.pos.y += player_data.move_dir.y * player_def.move_speed;
}

void frame_update() {
    player_data.move_input = (Vec2){0};
    if (is_key_on(SCANCODE_W)) player_data.move_input.y += 1;
    if (is_key_on(SCANCODE_A)) player_data.move_input.x -= 1;
    if (is_key_on(SCANCODE_S)) player_data.move_input.y -= 1;
    if (is_key_on(SCANCODE_D)) player_data.move_input.x += 1;

    player_data.move_dir = player_data.move_input;
    float len = sqrt((player_data.move_dir.x * player_data.move_dir.x) +
                     (player_data.move_dir.y * player_data.move_dir.y));
    if (len > 0.0001f) {
        player_data.move_dir.x /= len;
        player_data.move_dir.y /= len;
    } else {
        player_data.move_dir.x = 0.0f;
        player_data.move_dir.y = 0.0f;
    }
}

void render_update() {
    img_get_drawer_ptr(&image_sys, player_data.drawer)->srect = (Rect){0, 0, 32, 32};
    img_feed_drawer_world(&image_sys, player_data.drawer, player_data.pos, 0, 1);

    SDL_SetRenderDrawColor(sdl_renderer, 255, 255, 255, 255);
    SDL_RenderClear(sdl_renderer);

    img_draw(&image_sys, player_data.drawer);

    SDL_RenderPresent(sdl_renderer);
}
