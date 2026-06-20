import common;

import context;
import system;

import input;
import sprite;
import player;

import spines;

void logic_update();
void frame_update();
void render_update();

int main() {
    SpinesContext cxt;
    stdf::path filepath = asset_path / "data/data_asset.txt";
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open file " << filepath << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string config_content = buffer.str();
    file.close();

    auto char_count = config_content.size();
    auto line_count = std::count(config_content.begin(), config_content.end(), '\n');

    auto start_time = std::chrono::high_resolution_clock::now();
    auto err = cxt.parse(config_content);
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end_time - start_time;

    if (!err.type) {
        std::cout << "Successfully parsed '" << filepath << "' in " << elapsed_ms.count() << " ms.\n\n";
        cxt.print_debug();
    }
    else {
        std::cout << "Error parse: " << err.type << " at "
                  << err.line << ":" << err.column << std::endl;
    }

    cxt.destroy();
    return 0;

    window_width = 1280;
    window_height = 720;
    tps = 50;
    pixel_size = 3;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "Failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    if (!SDL_CreateWindowAndRenderer(
            "BurningFloor",
            window_width,
            window_height,
            0,
            &window,
            &renderer)) {
        std::cerr << "Failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    texture_sys.init(asset_path, 32);
    sprite_sys.init(64);

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

        cur_time_sec += dt;
        logic_update_accumulator += dt;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        pump_input();

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

    destroy_player();

    texture_sys.destroy();
    sprite_sys.destroy();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
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

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    sprite_sys.render_all_sprites();

    SDL_RenderPresent(renderer);
}
