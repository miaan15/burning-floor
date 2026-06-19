module;

export module context;

import common;

export struct GlobalContext {
    int window_width, window_height;
    SDL_Window *swindow;
    SDL_Renderer *renderer;
};

export GlobalContext global_context;
