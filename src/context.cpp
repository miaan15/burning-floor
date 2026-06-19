module;

export module context;

import common;

export struct GlobalContext {
    int window_width = 0, window_height = 0;
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;

    int tps = 0;
    int pixel_size = 0;

    double cur_time_sec;
    double logic_update_alpha;
};

export GlobalContext global_context{};
