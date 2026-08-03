#pragma once

#include "alloc/arena.h"
#include <SDL3/SDL_render.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

extern Arena global_ar;

extern SDL_Texture **texs;
extern size_t texs_len;
