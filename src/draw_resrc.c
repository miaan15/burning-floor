#include "draw_resrc.h"
#include "context.h"
#include "log.h"
#include <assert.h>
#include <stdalign.h>

// Texture
SDL_Texture **textures = NULL;
size_t textures_cap = 0;
size_t textures_len = 0;

void texture_init(size_t cap) {
    assert(!textures);
    textures_cap = cap;
    textures = arena_alloc(&global_ar, cap * sizeof(SDL_Texture *), alignof(SDL_Texture *));
    textures_len = 0;

    // stub
    SDL_Surface *stub_surf = SDL_CreateSurface(32, 32, SDL_PIXELFORMAT_RGBA8888);
    Uint32 black = SDL_MapSurfaceRGBA(stub_surf, 0, 0, 255, 255);
    Uint32 pink = SDL_MapSurfaceRGBA(stub_surf, 255, 0, 255, 255);
    SDL_Rect rect[4] = {{ 0,  0, 16, 16 },
                        { 16, 0, 16, 16 },
                        { 0, 16, 16, 16 },
                        { 16,16, 16, 16 }};
    SDL_FillSurfaceRect(stub_surf, &rect[0], pink);
    SDL_FillSurfaceRect(stub_surf, &rect[1], black);
    SDL_FillSurfaceRect(stub_surf, &rect[2], black);
    SDL_FillSurfaceRect(stub_surf, &rect[3], pink);

    textures[textures_len++] = SDL_CreateTextureFromSurface(renderer, stub_surf);

    SDL_DestroySurface(stub_surf);

    log_debug("New Texures Resources from %p to %p: cap = %zu",
            textures, (char *)textures + textures_cap, textures_cap);
}

size_t texture_new(const char *path, SDL_Renderer *renderer, SDL_ScaleMode scalemode) {
    assert(textures);
    if (textures_len >= textures_cap) {
        log_err("texture_new(): too much texture => stub");
        return 0;
    }

    SDL_Surface *surf = SDL_LoadPNG(path);
    if (!surf) {
        log_err("texture_new(): try load surface from %s but %s => return stub",
                path, SDL_GetError());
        return 0;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_DestroySurface(surf);
    SDL_SetTextureScaleMode(tex, scalemode);
    if (!tex) {
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
sprite *sprites = NULL;
size_t sprites_cap = 0;
size_t sprites_len = 0;

void sprite_init(size_t cap) {
    assert(!sprites);
    sprites_cap = cap;
    sprites = arena_alloc(&global_ar, cap * sizeof(sprite), alignof(sprite));
    sprites_len = 0;

    // stub
    sprites[sprites_len++] = (sprite){ 0, (rect){0, 0, 32, 32} };

    log_debug("New Sprites Resources from %p to %p: cap = %zu",
            sprites, (char *)sprites + sprites_cap, sprites_cap);
}

size_t sprite_new(u32 tex, rect srect) {
    assert(sprites);
    if (sprites_len >= sprites_cap) {
        log_err("sprite_new(): too much sprite => stub");
        return 0;
    }

    size_t idx = sprites_len;
    sprites[sprites_len++] = (sprite){ tex, srect };

    log_debug("Create Sprite [%zu]: tex = [%u], srect = %.0f %.0f %.0f %.0f",
            idx, tex, srect.x, srect.y, srect.w, srect.h);

    return idx;
}
