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
    Vec2 *center;
} DrawerHook;

extern Sprite *sprite_list;

extern Pool drawer_pool;
extern Pool drawer_hook_pool;

extern float drawer_zoom;
extern float drawer_scale;

void draw_init(float zoom, float scale);

void draw_update_hook(DrawerHook *hook);

void draw(Drawer *drawer);
