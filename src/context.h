#pragma once

#include "alloc/arena.h"
#include "macro.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern int window_w, window_h;

extern arena omni_arena;
extern arena tick_arenas[2];
extern size_t tick_arena_cur_idx;
extern arena *tick_arena;

extern const bool *keyb_state;
extern bool *last_keyb_state;

extern u32 time_ms;
extern u32 deltatime_ms;
extern f32 time_s;
extern f32 deltatime_s;

extern u32 ticks_cnt;
extern u32 tick_delta_ms;
extern f32 tick_alpha;
extern bool tick_flag;
