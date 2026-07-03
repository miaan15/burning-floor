#pragma once

#include <SDL3/SDL_render.h>
#include <stdalign.h>

#include "allocator/allocator.h"

typedef struct {
    SDL_Texture *tex;
    SDL_FRect rect;
} Sprite;

typedef struct {
    size_t sprite;
    SDL_FRect drect;
    SDL_FlipMode flip;
} SpriteDrawer;

typedef struct {
    Arena arena;

    Sprite *sprites;
    size_t sprites_len;
    size_t sprites_cap;

    SpriteDrawer *drawers;
    size_t drawers_offset;
    size_t drawers_count;
    size_t drawers_cap;
    size_t drawers_next_id;
    void *drawers_deleted;
} SpriteSys;

void spr_sys_init(SpriteSys *sys, size_t sprites_cap, size_t drawers_cap);
void spr_sys_destroy(SpriteSys *sys);

size_t spr_make_sprite(SpriteSys *sys, SDL_Texture *texture, SDL_FRect *rect);
Sprite spr_get_sprite(SpriteSys *sys, size_t index);
Sprite *spr_get_sprite_ptr(SpriteSys *sys, size_t index);

size_t spr_make_drawer(SpriteSys *sys, size_t sprite);
void spr_remove_drawer(SpriteSys *sys, size_t index);
SpriteDrawer spr_get_drawer(SpriteSys *sys, size_t index);
SpriteDrawer *spr_get_drawer_ptr(SpriteSys *sys, size_t index);

void spr_update_drawer(SpriteSys *sys, size_t index,
                       SDL_FPoint pos, bool flip, float scale);

void spr_render(SpriteSys *sys, size_t index);
