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
} DrHook;

typedef struct {
    size_t cap;

    Pool drawer_pool;
    Pool hook_pool;

    // configs
    float zoom, scale;
} DrawSys;

typedef struct {
    Drawer *drawer;
    DrHook *hook;
} DrawerAndHook;

// Sprite
extern size_t sprite_cap;
extern Sprite *sprites;
extern size_t sprites_len;

extern Sprite *sprite_stub;

void sprite_init(size_t cap);

Sprite *sprite_new(SDL_Texture *tex, SDL_FRect *srect);

// Draw
extern DrawSys draw_sys;

extern Drawer *drawer_stub;
extern DrHook *drawer_hook_stub;

void draw_init(size_t cap, float zoom, float scale);

DrawerAndHook draw_new(Sprite *sprite, SDL_FRect *drect);

void draw_update();

void draw();
