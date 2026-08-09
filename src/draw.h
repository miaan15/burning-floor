#pragma once

#include "macro.h"
#include "math/vec.h"
#include <SDL3/SDL_render.h>

typedef struct {
    SDL_FRect srect;
    SDL_FRect drect;
    u32 tex;
} texdr;

typedef struct __attribute__((packed)) {
    u64 z   : 8;
    u64 tex : 32;
    u64 idk : 24;
} texdr_meta;

typedef struct {
    SDL_FRect rect;
    u8 r, g, b, a;
} rectdr;

typedef struct __attribute__((packed)) {
    u64 r : 8;
    u64 g : 8;
    u64 b : 8;
    u64 a : 8;
    u64 idk : 32;
} rectdr_meta;

extern texdr *texdrs;
extern texdr_meta *texdr_metas;
extern size_t texdrs_cap;
extern size_t texdrs_len;

extern rectdr *rectdrs;
extern rectdr_meta *rectdr_metas;
extern size_t rectdrs_cap;
extern size_t rectdrs_len;

void draw_init(size_t texdrs_cap, size_t rectdrs_cap);

void draw_sprite_wpos(size_t sprite, vec2 pos, i8 z, vec2 center, vec2 scale);

void draw_rect_wpos(vec2 pos, vec2 size, vec2 center, u8 r, u8 g, u8 b, u8 a);

void draw_present();
