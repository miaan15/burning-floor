#include "img.h"

#include "log/log.h"
#include <assert.h>
#include <cglm/cglm.h>

// =============================================================================
// IMAGE
void img_mng_init(ImgMng *mng, size_t cap) {
    ++cap; // stub
    mng->raw = malloc(cap * sizeof(ImgIns));
    mng->cap = cap;
    mng->len = 0;

    // stub
    mng->raw[0] = (ImgIns){0}; // FIXME
    ++mng->len;
}

void img_mng_destroy(ImgMng *mng) {
    for (size_t i = 0; i < mng->len; ++i) SDL_DestroyTexture(mng->raw[i].tex);
    free(mng->raw);
    mng->cap = mng->len = 0;
}

size_t img_new(ImgMng *mng, const char *path,
               SDL_Renderer *renderer, SDL_ScaleMode scalemode) {
    if (mng->len >= mng->cap) {
        log_err("img_new(): full texture capacity => return stub");
        return 0;
    }

    SDL_Surface *surf = SDL_LoadPNG(path);
    if (!surf) {
        log_err("img_new(): try load surface from %s but %s => return stub",
                path, SDL_GetError());
        return 0;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureScaleMode(tex, scalemode);
    if (!tex) {
        log_err("img_new(): try load texture from %s but %s => return stub",
                path, SDL_GetError());
        SDL_DestroySurface(surf);
        return 0;
    }

    mng->raw[mng->len] = (ImgIns){tex, surf->w, surf->h};

    log_debug("Made a new image %zu: dir: %s, scale mode: %d; size: %dx%d",
              mng->len, path, scalemode, surf->w, surf->h);

    SDL_DestroySurface(surf);

    return mng->len++;
}

ImgIns *img_get(ImgMng *mng, size_t img) {
    if (img >= mng->len) {
        log_err("img_get(): image %zu is dead or invalid => return stub", img);
        return mng->raw;
    }
    return &mng->raw[img];
}

// =============================================================================
// SPRITE
void spr_mng_init(SprMng *mng, size_t cap, ImgMng *img_mng) {
    ++cap; // stub

    mng->raw = malloc(cap * sizeof(SprIns));
    mng->cap = cap;
    mng->len = 0;

    mng->img_mng = img_mng;

    // stub
    mng->raw[0] = (SprIns){0}; // FIXME
    ++mng->len;
}

void spr_mng_destroy(SprMng *mng) {
    free(mng->raw);
    mng->cap = mng->len = 0;
}

size_t spr_new(SprMng *mng, size_t img, mat2 rect) {
    mng->raw[mng->len].img = img;
    glm_mat2_copy(rect, mng->raw[mng->len].rect);
    log_debug("Made a new sprite %zu: img: %zu, srect: %.1f : %.1f - %.1f x %.1f",
              mng->len, img, rect[0][0], rect[0][1], rect[1][0], rect[1][1]);
    return mng->len++;
}

SprIns *spr_get(SprMng *mng, size_t spr) {
    if (spr >= mng->len) {
        log_err("spr_get(): sprite %zu is dead or invalid => return stub", spr);
        return mng->raw;
    }
    return &mng->raw[spr];
}

// =============================================================================
// DRAWER
void drwr_mng_init(DrwrMng *mng, size_t cap, SprMng *spr_mng, float scale, int pixel_size) {
    ++cap; // stub

    size_t arena_size = 0;
    arena_size += poola_msize(sizeof(DrwrIns), cap);
    arena_size = align_up(arena_size, alignof(DrwrHook));
    arena_size += poola_msize(sizeof(DrwrHook), cap);
    arena_init(&mng->arena, arena_size);

    poola_init(&mng->drwr_pool, &mng->arena, sizeof(DrwrIns), alignof(DrwrIns), cap);
    poola_init(&mng->hook_pool, &mng->arena, sizeof(DrwrHook), alignof(DrwrHook), cap);

    mng->spr_mng = spr_mng;
    if (pixel_size < 1) {
        log_warn("drwr_mng_init(): pixel_size must be >= 1 (which was %d)", pixel_size);
        pixel_size = 1;
    }
    mng->scale = scale;
    mng->pixel_size = pixel_size;

    // stub
    size_t drwr_stub = poola_new(&mng->drwr_pool);
    *(DrwrIns *)poola_get(&mng->drwr_pool, drwr_stub) = (DrwrIns){0}; // FIXME
    size_t hook_stub = poola_new(&mng->hook_pool);
    *(DrwrHook *)poola_get(&mng->hook_pool, hook_stub) = (DrwrHook){0}; // FIXME
}

void drwr_mng_destroy(DrwrMng *mng) {
    arena_destroy(&mng->arena);
}

void drwr_mng_update(DrwrMng *mng) {
    // TODO support screen pos
    poola_for(&mng->drwr_pool, drwr, ptr) {
        DrwrIns *drwr_ins = (DrwrIns *)ptr;
        DrwrHook *hook_ins = (DrwrHook *)poola_get(&mng->hook_pool, drwr);

        switch (hook_ins->type) {
        case DRWR_HOOK_WPOS: {
            drwr_feed_wpos(mng, drwr,
                           hook_ins->wpos_pos, hook_ins->wpos_offset,
                           hook_ins->wpos_center, hook_ins->wpos_rot,
                           hook_ins->wpos_flip, hook_ins->wpos_scale);
        } break;

        case DRWR_HOOK_NONE: break;
        default: break;
        }
    }
}

// TODO: add z level; pixel size
void drwr_mng_draw(DrwrMng *mng, SDL_Renderer *renderer, SDL_Window *window) {
    int window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    SprMng *_spr_mng = mng->spr_mng;
    ImgMng *_img_mng = _spr_mng->img_mng;

    poola_for(&mng->drwr_pool, drwr, ptr) {
        DrwrIns *ins = (DrwrIns *)ptr;

        assert((size_t)ins->spr < _spr_mng->len);
        SprIns spr_ins = _spr_mng->raw[(size_t)ins->spr];

        assert((size_t)spr_ins.img < _img_mng->len);
        ImgIns img_ins = _img_mng->raw[(size_t)spr_ins.img];

        SDL_FRect srect = (SDL_FRect){ spr_ins.rect[0][0], spr_ins.rect[0][1],
                                       spr_ins.rect[1][0], spr_ins.rect[1][1] };

        SDL_FRect drect = (SDL_FRect){ ins->drect[0][0], ins->drect[0][1],
                                       ins->drect[1][0], ins->drect[1][1] };
        drect.x *= mng->scale;
        drect.y *= mng->scale;
        drect.x = (int)drect.x * mng->pixel_size;
        drect.y = (int)drect.y * mng->pixel_size;

        drect.w *= mng->scale;
        drect.h *= mng->scale;
        drect.w = (int)drect.w * mng->pixel_size;
        drect.h = (int)drect.h * mng->pixel_size;

        drect.y = window_h - drect.y - drect.h;

        SDL_FPoint center = (SDL_FPoint){ ins->center[0], ins->drect[1][1] - ins->center[1] };
        center.x *= mng->pixel_size * mng->scale;
        center.y *= mng->pixel_size * mng->scale;

        SDL_RenderTextureRotated(renderer, img_ins.tex, &srect, &drect,
                                 (double)ins->rot, &center, ins->flip);
    }
}

size_t drwr_new(DrwrMng *mng, size_t spr, size_t z) {
    size_t drwr = poola_new(&mng->drwr_pool);
    size_t hook = poola_new(&mng->hook_pool);
    assert(drwr == hook);
    if (drwr < 0) {
        log_err("drwr_new(): cannot create new drawer => return stub");
        return 0;
    }
    DrwrIns *drwr_ins = (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
    drwr_ins->spr = spr;
    drwr_ins->z = z;
    drwr_ins->active = true;
    // DrwrHook *hook_ins = (DrwrHook *)poola_get(&mng->hook_pool, hook);

    log_debug("Made a new drawer %zu: sprite: %zu, z: %d", drwr, spr, z);

    return drwr;
}

void drwr_remv(DrwrMng *mng, size_t drwr) {
    if (!poola_alive(&mng->drwr_pool, drwr)) {
        log_err("drwr_remv(): drawer %zu is dead or invalid", drwr);
        return;
    }
    poola_remv(&mng->drwr_pool, drwr);
    poola_remv(&mng->hook_pool, drwr);
}

DrwrIns *drwr_get(DrwrMng *mng, size_t drwr) {
    if (!poola_alive(&mng->drwr_pool, drwr)) {
        log_err("drwr_get(): drawer %zu is dead or invalid => return stub", drwr);
        return (DrwrIns *)poola_get(&mng->drwr_pool, 0);
    }
    return (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
}

DrwrHook *drwr_get_hook(DrwrMng *mng, size_t drwr) {
    if (!poola_alive(&mng->hook_pool, drwr)) {
        log_err("drwr_get_hook(): drawer %zu is dead or invalid => return stub", drwr);
        return (DrwrHook *)poola_get(&mng->hook_pool, 0);
    }
    return (DrwrHook *)poola_get(&mng->hook_pool, drwr);
}

void drwr_feed_wpos(DrwrMng *mng, size_t drwr,
                    vec2 pos, vec2 offset, vec2 center,
                    vec2 rot, int *flip, vec2 scale) {
    if (!poola_alive(&mng->drwr_pool, drwr)) {
        log_err("drwr_feed_wpos(): drawer %zu is dead or invalid", drwr);
        return;
    }

    DrwrIns *drwr_ins = (DrwrIns *)poola_get(&mng->drwr_pool, drwr);

    vec2 _pos = {0, 0};
    if (pos) glm_vec2_add(_pos, pos, _pos);
    if (offset) glm_vec2_add(_pos, offset, _pos);

    SprIns *spr_ins = spr_get(mng->spr_mng, drwr_ins->spr);
    vec2 _size; glm_vec2_copy(spr_ins->rect[1], _size);
    if (scale) glm_vec2_scale(_size, *scale ,_size);

    vec2 _center = {0, 0};
    if (center) {
        _center[0] = center[0] * _size[0];
        _center[1] = center[1] * _size[1];
    }

    glm_vec2_sub(_pos, _center, drwr_ins->drect[0]);
    glm_vec2_copy(_size, drwr_ins->drect[1]);

    drwr_ins->flip = SDL_FLIP_NONE;
    if (flip) drwr_ins->flip = *flip;

    drwr_ins->rot = 0;
    if (rot) drwr_ins->rot = *rot;

    glm_vec2_copy(_center, drwr_ins->center);
}

float _drwr_hook_wpos_center_mid[2] = { .5, .5 };
int _drwr_hook_wpos_flip_horizontal = SDL_FLIP_HORIZONTAL;
int _drwr_hook_wpos_flip_vertical = SDL_FLIP_VERTICAL;
