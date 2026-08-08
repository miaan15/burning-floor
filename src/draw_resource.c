#include "draw_resource.h"
#include "context.h"
#include "log.h"
#include <assert.h>
#include <stdalign.h>

// Texture
size_t texture_cap = 0;
SDL_Texture **textures = NULL;
size_t textures_len = 0;

void texture_init(size_t cap) {
    texture_cap = cap;
    textures = arena_alloc(&global_ar, cap * sizeof(SDL_Texture *), alignof(SDL_Texture *));
    textures_len = 0;

    // stub
    textures[textures_len++] = NULL; // FIXME create unique stub texture 32x32

    log_debug("New Texures Resources from %p to %p: cap = %zu",
            textures, (char *)textures + texture_cap, texture_cap);
}

u32 texture_new(const char *path, SDL_Renderer *renderer, SDL_ScaleMode scalemode) {
    if (unlikely(textures_len >= texture_cap)) {
        log_err("texture_new(): too much texture => stub");
        return 0;
    }

    SDL_Surface *surf = SDL_LoadPNG(path);
    if (unlikely(!surf)) {
        log_err("texture_new(): try load surface from %s but %s => return stub",
                path, SDL_GetError());
        return 0;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    SDL_SetTextureScaleMode(tex, scalemode);
    if (unlikely(!tex)) {
        log_err("texture_new(): try load texture from %s but %s => return stub",
                path, SDL_GetError());
        return 0;
    }

    size_t idx = textures_len;
    textures[textures_len++] = tex;

    log_debug("Create Texture [%zu]: dir = %s, scale mode = %d; size = %d %d",
            idx, path, scalemode, tex->w, tex->h);

    return (u32)idx;
}

// Sprite
size_t sprite_cap = 0;
Sprite *sprites = NULL;
size_t sprites_len = 0;

void sprite_init(size_t cap) {
    sprite_cap = cap;
    sprites = arena_alloc(&global_ar, cap * sizeof(Sprite), alignof(Sprite));
    sprites_len = 0;

    // stub
    sprites[sprites_len++] = (Sprite){ 0, (SDL_FRect){0, 0, 32, 32} };

    log_debug("New Sprites Resources from %p to %p: cap = %zu",
            sprites, (char *)sprites + sprite_cap, sprite_cap);
}

u32 sprite_new(u32 tex, SDL_FRect *srect) {
    if (unlikely(sprites_len >= sprite_cap)) {
        log_err("sprite_new(): too much sprite => stub");
        return 0;
    }

    size_t idx = sprites_len;
    sprites[sprites_len++] = (Sprite){tex, *srect};

    log_debug("Create Sprite [%zu]: tex = [%u], srect = %.0f %.0f %.0f %.0f",
            idx, tex, srect->x, srect->y, srect->w, srect->h);

    return idx;
}
