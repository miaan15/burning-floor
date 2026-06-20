module;

export module context;

import common;

export int window_width = 0;
export int window_height = 0;
export SDL_Window *window = nullptr;
export SDL_Renderer *renderer = nullptr;

export int tps = 0;
export int pixel_size = 0;

export double cur_time_sec;
export double logic_update_alpha;
