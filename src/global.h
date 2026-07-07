#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include "spines/spines.h"
#include "image/image.h"

extern int window_width;
extern int window_height;
extern SDL_Window *sdl_window;
extern SDL_Renderer *sdl_renderer;

extern int tps;
extern int pixel_size;

extern double cur_time;
extern double logic_update_alpha;

extern spn_Context cfg_context;

extern ImageSys image_sys;
