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

typedef struct {
    SDL_FRect rect;
    u8 r, g, b, a;
} RectDrawer;

typedef struct {
    u64 r : 8;
    u64 g : 8;
    u64 b : 8;
    u64 a : 8;
    u64 idk : 32;
} RectDrawerMeta;

extern Drawer *drawers_raw;
extern DrawerMeta *drawer_metas_raw;
extern size_t drawers_len;

extern RectDrawer *rect_drawers_raw;
extern RectDrawerMeta *rect_drawer_metas_raw;
extern size_t rect_drawers_len;

void draw_init(size_t cap, size_t rect_cap);

void draw_sprite_wpos(uint32_t sprite, Vec2 pos, int8_t z, Vec2 center, Vec2 scale);

void draw_rect_wpos(Vec2 pos, Vec2 size, Vec2 center, u8 r, u8 g, u8 b, u8 a);

void draw();
