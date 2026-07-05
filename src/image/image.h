#pragma once

#include <SDL3/SDL_render.h>

#include "allocator/allocator.h"

typedef struct {
    SDL_Texture *tex;
    SDL_FRect srect;
    SDL_FRect drect;
    SDL_FlipMode flip;
} ImageDrawer;

typedef struct {
    int w, h;
} ImageTexData;

typedef struct {
    Arena arena;

    SDL_Texture **texs;
    ImageTexData *tex_datas;
    size_t texs_len;
    size_t texs_cap;

    ImageDrawer *drawers;
    size_t drawers_offset;
    size_t drawers_count;
    size_t drawers_cap;
    size_t drawers_next_id;
    void *drawers_deleted;
} ImageSys;

void img_sys_init(ImageSys *sys, size_t image_cap, size_t drawer_cap);
void img_sys_destroy(ImageSys *sys);

size_t img_load_tex(ImageSys *sys, const char *path, SDL_ScaleMode scalemode);
SDL_Texture *img_get_tex(ImageSys *sys, size_t index);

size_t img_make_drawer(ImageSys *sys, size_t image);
void img_remove_drawer(ImageSys *sys, size_t drawer);

ImageDrawer *img_get_drawer_ptr(ImageSys *sys, size_t drawer);

void img_feed_drawer_world(ImageSys *sys, size_t drawer,
                           SDL_FPoint pos, bool flip, float scale);

void img_draw(ImageSys *sys, size_t drawer);
