import common;

import context;

import input;
import player;

void logic_update();
void frame_update();
void render_update();

int main() {
    global_context.window_width = 1280;
    global_context.window_height = 720;
    global_context.tps = 50;
    global_context.pixel_size = 3;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!SDL_CreateWindowAndRenderer(
            "BurningFloor",
            global_context.window_width,
            global_context.window_height,
            0,
            &global_context.window,
            &global_context.renderer)) {
        std::cerr << "Failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    init_input();

    init_player();

    bool running = true;
    u64 last_time_ns = SDL_GetTicksNS();
    double logic_update_accumulator = 0.0;
    while (running) {
        u64 cur_time_ns = SDL_GetTicksNS();
        double dt = (double)(cur_time_ns - last_time_ns) / 1000000000.0;
        last_time_ns = cur_time_ns;

        if (dt > 1) dt = 1; // still need 1 fps

        global_context.cur_time_sec += dt;
        logic_update_accumulator += dt;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        pump_input();

        const double logic_update_dt = (double)1.0 / global_context.tps;
        while (logic_update_accumulator >= logic_update_dt) {
            logic_update();
            logic_update_accumulator -= logic_update_dt;
        }

        global_context.logic_update_alpha =
            logic_update_accumulator / logic_update_dt;

        frame_update();

        render_update();
    }

    destroy_player();

    SDL_DestroyRenderer(global_context.renderer);
    SDL_DestroyWindow(global_context.window);
    SDL_Quit();
}

void logic_update() {
    logic_update_player();
}

void frame_update() {
    frame_update_player();
}

void render_update() {
    render_update_player();

    SDL_SetRenderDrawColor(global_context.renderer, 255, 255, 255, 255);
    SDL_RenderClear(global_context.renderer);

    SDL_RenderTextureRotated(
        global_context.renderer,
        player_data.sprite.texture,
        &player_data.sprite.src_rect,
        &player_data.sprite.dest_rect,
        0,
        nullptr,
        player_data.sprite.flip);

    SDL_RenderPresent(global_context.renderer);
}
