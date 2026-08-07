#pragma once

#include "macro.h"
#include <SDL3/SDL_render.h>

typedef struct {
    u32 tex;
    SDL_FRect srect;
} Sprite;

// Texture
extern size_t texture_cap;
extern SDL_Texture **textures;
extern size_t textures_len;

void texture_init(size_t cap);
u32 texture_new(const char *path, SDL_Renderer *renderer, SDL_ScaleMode scalemode);

// Sprite
extern size_t sprite_cap;
extern Sprite *sprites;
extern size_t sprites_len;

void sprite_init(size_t cap);
u32 sprite_new(u32 tex, SDL_FRect *srect);
