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

    Vec2 last_pos;
} Drawer;

typedef struct {
    bool active;
    Vec2 *pos;
    Vec2 *size;
    Vec2 *center;
} DrawerHook;

typedef struct {
    size_t sprite_cap;
    size_t drawer_cap;

    Sprite *sprites;
    size_t sprites_len;

    Pool drawer_pool;
    Pool hook_pool;

    // configs
    float zoom, scale;
} DrawSys;

typedef struct {
    Drawer *drawer;
    DrawerHook *hook;
    char meta;
} DrawerResult;

extern DrawSys draw_sys;

extern Sprite *sprite_stub;
extern Drawer *drawer_stub;

void draw_init(DrawSys *sys, size_t sprite_cap, size_t drawer_cap, float zoom, float scale);

Sprite *draw_new_sprite(DrawSys *sys, SDL_Texture *tex, SDL_FRect *srect);
DrawerResult draw_new_drawer(DrawSys *sys, Sprite *sprite, SDL_FRect *drect);

void draw_update(DrawSys *sys);

void draw(DrawSys *sys);
