#pragma once

#include <SDL3/SDL_render.h>

typedef struct {
    SDL_Texture **texs;
    size_t len;
    size_t cap;
} ImageSys;

void img_sys_init(ImageSys *sys, size_t cap);
void img_sys_destroy(ImageSys *sys);

size_t img_load(ImageSys *sys, const char *path);
SDL_Texture *img_get(ImageSys *sys, size_t index);
