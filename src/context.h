#pragma once

#include "alloc/arena.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern int window_w, window_h;

extern Arena global_ar;

extern const bool *keyb_state;
extern bool *last_keyb_state;

extern uint64_t time_ms;
extern uint64_t deltatime_ms;
extern float time_s;
extern float deltatime_s;

extern uint64_t ticks_cnt;
extern uint64_t tick_delta_ms;
extern float tick_alpha;
extern bool tick_flag;
