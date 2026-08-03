#pragma once

#include "alloc/pool.h"
#include "math/vec.h"
#include <SDL3/SDL_render.h>

typedef struct {
    SDL_Texture *tex;
    SDL_FRect srect;
} Sprite;

typedef struct {
    Sprite *sprite;
    SDL_FRect drect;
} Drawer;

typedef struct {
    Vec2 *pos;
} DrawerHook;

extern Sprite *sprite_list;

extern Pool drawer_pool;
extern Pool drawer_hook_pool;

void draw_init();
