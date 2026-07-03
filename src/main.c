#include <SDL3/SDL.h>

#include "macro.h"
#include "common.h"
#include "global.h"
#include "log/log.h"
#include "input/input.h"

int window_width = 1280;
int window_height = 720;
SDL_Window *sdl_window = NULL;
SDL_Renderer *sdl_renderer = NULL;

int tps = 50;
int pixel_size = 10;

int cur_time = 0;
int logic_update_alpha = 0;

void logic_update();
void frame_update();

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_err("%s", SDL_GetError());
        return 1;
    }

    if (!SDL_CreateWindowAndRenderer("BurningFloor",
                                     window_width, window_height,
                                     0,
                                     &sdl_window, &sdl_renderer)) {
        log_err("%s", SDL_GetError());
        return 1;
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
    }

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}

void logic_update() {}

void frame_update() {}
