#pragma once

#include "math/mat.h"
#include "macro.h"
#include <SDL3/SDL_render.h>

// Texture
extern SDL_Texture **textures;
extern size_t textures_cap;
extern size_t textures_len;

void texture_init(size_t cap);
size_t texture_new(const char *path, SDL_Renderer *renderer, SDL_ScaleMode scalemode);

// Sprite
typedef struct {
    u32 tex;
    rect srect;
} sprite;

extern sprite *sprites;
extern size_t sprites_cap;
extern size_t sprites_len;

void sprite_init(size_t cap);
size_t sprite_new(u32 tex, rect srect);
