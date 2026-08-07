#pragma once

#include "macro.h"
#include "math/vec.h"
#include <SDL3/SDL_render.h>

typedef struct {
    SDL_FRect srect;
    SDL_FRect drect;
    u32 tex;
} Drawer;

typedef struct __attribute__((packed)) {
    u64 z   : 8;
    u64 tex : 32;
    u64 idk : 24;
} DrawerMeta;

extern Drawer *drawers_raw;
extern DrawerMeta *drawer_metas_raw;
extern size_t drawers_len;

void draw_init(size_t cap);

void draw_sprite_wpos(uint32_t sprite, Vec2 pos, int8_t z, Vec2 center, Vec2 scale);

void draw();
