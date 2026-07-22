#pragma once

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include "ett/ett.h"
#include "img/img.h"
#include "spines/spines.h"

extern int window_width;
extern int window_height;
extern SDL_Window *sdl_window;
extern SDL_Renderer *sdl_renderer;

extern int tps;
extern int pixel_size;

extern float cur_frame_time;
extern float cur_logic_time;
extern float logic_update_alpha;

extern spn_Context cfg_context;

extern EttMng ett_mng;

extern ImgMng img_mng;
extern SprMng spr_mng;
extern DrwrMng drwr_mng;
