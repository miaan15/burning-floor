#include "image.h"

#include <stdlib.h>

#include "log/log.h"
#include "global.h"

void img_sys_init(ImageSys *sys, size_t cap) {
    sys->texs = (SDL_Texture **)malloc(cap * sizeof(SDL_Texture *));
    sys->len = 0;
    sys->cap = cap;
}

void img_sys_destroy(ImageSys *sys) {
    for (size_t i = 0; i < sys->len; ++i) {
        SDL_DestroyTexture(sys->texs[i]);
    }
    free(sys->texs);
    sys->len = sys->cap = 0;
}

size_t img_load(ImageSys *sys, const char *path) {
    if (sys->len >= sys->cap) return 0;

    SDL_Surface *surf = SDL_LoadPNG(path);
    if (!surf) {
        log_warn("texture_sys_load(): %s => Return 0", SDL_GetError());
        return 0;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(sdl_renderer, surf);
    SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surf);
    if (!tex) {
        log_warn("texture_sys_load(): %s => Return 0", SDL_GetError());
        return 0;
    }

    sys->texs[sys->len] = tex;
    return sys->len++;
}

SDL_Texture *img_get(ImageSys *sys, size_t index) {
    if (index > sys->len) {
        log_warn("texture_sys_get(): index out of bounds => Return element-0");
        return sys->texs[0];
    }
    return sys->texs[index];
}
