#include "img.h"

#include "log/log.h"
#include "macro.h"
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
    assert(mng->raw);
    if (unlikely(mng->len >= mng->cap)) {
        log_err("img_new(): full texture capacity => return stub");
        return 0;
    }

    SDL_Surface *surf = SDL_LoadPNG(path);
    if (unlikely(!surf)) {
        log_err("img_new(): try load surface from %s but %s => return stub",
                path, SDL_GetError());
        return 0;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureScaleMode(tex, scalemode);
    if (unlikely(!tex)) {
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
    assert(mng->raw);
    if (unlikely(img >= mng->len)) {
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
    assert(mng->raw);
    mng->raw[mng->len].img = img;
    glm_mat2_copy(rect, mng->raw[mng->len].srect);
    log_debug("Made a new sprite %zu: img: %zu, srect: %.0f:%.0f-%.0fx%.0f",
              mng->len, img, rect[0][0], rect[0][1], rect[1][0], rect[1][1]);
    return mng->len++;
}

SprIns *spr_get(SprMng *mng, size_t spr) {
    assert(mng->raw);
    if (unlikely(spr >= mng->len)) {
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
    arena_size += pool_msize(sizeof(DrwrIns), cap);
    arena_size = align_up(arena_size, alignof(DrwrHook));
    arena_size += pool_msize(sizeof(DrwrHook), cap);
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
    Key drwr_stub = poola_new(&mng->drwr_pool);
    *(DrwrIns *)poola_get(&mng->drwr_pool, drwr_stub) = (DrwrIns){0}; // FIXME

    Key hook_stub = poola_new(&mng->hook_pool);
    *(DrwrHook *)poola_get(&mng->hook_pool, hook_stub) = (DrwrHook){0}; // FIXME
}

void drwr_mng_destroy(DrwrMng *mng) {
    poola_destroy(&mng->drwr_pool);
    poola_destroy(&mng->hook_pool);
    arena_destroy(&mng->arena);
}

void drwr_mng_update(DrwrMng *mng) {
    // TODO support screen pos
    poola_for(&mng->drwr_pool, drwr, ptr) {
        DrwrIns *drwr_ins = (DrwrIns *)ptr;
        DrwrHook *hook_ins = (DrwrHook *)poola_get(&mng->hook_pool, drwr);

        switch (hook_ins->type) {
        case DRWR_HOOK_WPOS:
            drwr_feed_wpos(drwr_ins,
                           hook_ins->wpos_pos, hook_ins->wpos_offs,
                           hook_ins->wpos_centr, hook_ins->wpos_rot,
                           hook_ins->wpos_flip, hook_ins->wpos_scale);
        break;

        case DRWR_HOOK_SWPOS:
            drwr_feed_swpos(drwr_ins,
                            hook_ins->swpos_pos, hook_ins->swpos_rot,hook_ins->swpos_scale);
        break;

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

        SDL_FRect srect = (SDL_FRect){ spr_ins.srect[0][0], spr_ins.srect[0][1],
                                       spr_ins.srect[1][0], spr_ins.srect[1][1] };

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

        SDL_FPoint center = (SDL_FPoint){ ins->centr[0], ins->drect[1][1] - ins->centr[1] };
        center.x *= mng->pixel_size * mng->scale;
        center.y *= mng->pixel_size * mng->scale;

        SDL_RenderTextureRotated(renderer, img_ins.tex, &srect, &drect,
                                 (double)ins->rot, &center, ins->flip);
    }
}

Key drwr_new(DrwrMng *mng, size_t spr, int z) {
    assert(mng->arena.buffer);
    Key drwr = poola_new(&mng->drwr_pool);
    Key hook = poola_new(&mng->hook_pool);

    assert(drwr.idx == hook.idx && drwr.gen == hook.gen);

    if (unlikely(key2u64(drwr) == 0)) {
        log_err("drwr_new(): cannot create new drawer => return stub");
        return (Key){0, 0};
    }

    DrwrIns *drwr_ins = (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
    drwr_ins->spr = spr;
    drwr_ins->z = z;
    drwr_ins->active = true;

    SprIns *spr_ins = spr_get(mng->spr_mng, spr);
    glm_vec2_copy(spr_ins->srect[1], drwr_ins->srect_size);

    log_debug("Made a new drawer %u.%u: sprite: %zu, z: %zu", drwr.idx, drwr.gen, spr, z);

    return drwr;
}

void drwr_remv(DrwrMng *mng, Key drwr) {
    assert(mng->arena.buffer);
    if (unlikely(!poola_alive(&mng->drwr_pool, drwr))) {
        log_err("drwr_remv(): drawer %u.%u is dead or invalid", drwr.idx, drwr.gen);
        return;
    }
    poola_remv(&mng->drwr_pool, drwr);
    poola_remv(&mng->hook_pool, drwr);
}

DrwrIns *drwr_get(DrwrMng *mng, Key drwr) {
    assert(mng->arena.buffer);
    if (unlikely(!poola_alive(&mng->drwr_pool, drwr))) {
        log_err("drwr_get(): drawer %u.%u is dead or invalid => return stub", drwr.idx, drwr.gen);
        return (DrwrIns *)poola_get(&mng->drwr_pool, (Key){0, 0});
    }
    return (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
}

DrwrHook *drwr_get_hook(DrwrMng *mng, Key drwr) {
    assert(mng->arena.buffer);
    if (unlikely(!poola_alive(&mng->hook_pool, drwr))) {
        log_err("drwr_get_hook(): drawer %u.%u is dead or invalid => return stub", drwr.idx, drwr.gen);
        return (DrwrHook *)poola_get(&mng->hook_pool, (Key){0, 0});
    }
    return (DrwrHook *)poola_get(&mng->hook_pool, drwr);
}

void drwr_hook_set_wpos(DrwrMng *mng, Key drwr,
                        float *pos, float *offs, float *centr,
                        float *rot, int *flip, float *scale) {
    DrwrHook *hook = drwr_get_hook(mng, drwr);
    hook->type = DRWR_HOOK_WPOS;
    hook->wpos_pos = pos;
    hook->wpos_offs = offs;
    hook->wpos_centr = centr;
    hook->wpos_rot = rot;
    hook->wpos_flip = flip;
    hook->wpos_scale = scale;
    log_debug("Hook a drawer %u.%u with [wpos]: pos: %p, offs: %p, centr: %p, rot: %p, flip: %p, scale: %p",
              drwr.idx, drwr.gen, pos, offs, centr, rot, flip, scale);
}

void drwr_feed_wpos(DrwrIns *drwr_ins,
                    vec2 pos, vec2 offs, vec2 centr,
                    vec2 rot, int *flip, vec2 scale) {
    vec2 _pos = {0, 0};
    if (likely(pos)) glm_vec2_copy(pos, _pos);
    if (offs) glm_vec2_add(_pos, offs, _pos);

    vec2 _size; glm_vec2_copy(drwr_ins->srect_size, _size);
    if (scale) glm_vec2_scale(_size, *scale, _size);

    vec2 _centr = {0, 0};
    if (centr) {
        _centr[0] = centr[0] * _size[0];
        _centr[1] = centr[1] * _size[1];
    }

    glm_vec2_sub(_pos, _centr, drwr_ins->drect[0]);
    glm_vec2_copy(_size, drwr_ins->drect[1]);

    drwr_ins->flip = flip != NULL ? *flip : SDL_FLIP_NONE;
    drwr_ins->rot = rot != NULL ? *rot : 0;

    glm_vec2_copy(_centr, drwr_ins->centr);
}

void drwr_hook_set_swpos(DrwrMng *mng, Key drwr, float *pos, float *rot, float *scale) {
    DrwrHook *hook = drwr_get_hook(mng, drwr);
    hook->type = DRWR_HOOK_SWPOS;
    hook->swpos_pos = pos;
    hook->swpos_rot = rot;
    hook->swpos_scale = scale;
    log_debug("Hook a drawer %u.%u with [swpos]: pos: %p, rot: %p, scale: %p",
              drwr.idx, drwr.gen, pos, rot, scale);
}

void drwr_feed_swpos(DrwrIns *drwr_ins, float *pos, float *rot, float *scale) {
    vec2 _pos = {0, 0};
    if (likely(pos)) glm_vec2_copy(pos, _pos);

    vec2 _size; glm_vec2_copy(drwr_ins->srect_size, _size);
    if (scale) glm_vec2_scale(_size, *scale, _size);

    drwr_ins->drect[0][0] = _pos[0] - (_size[0] / 2);
    drwr_ins->drect[0][1] = _pos[1] - (_size[1] / 2);
    glm_vec2_copy(_size, drwr_ins->drect[1]);

    drwr_ins->rot = rot != NULL ? *rot : 0;

    drwr_ins->flip = SDL_FLIP_NONE;
    drwr_ins->centr[0] = .5;
    drwr_ins->centr[1] = .5;
}

int _drwr_hook_wpos_flip_horizontal = SDL_FLIP_HORIZONTAL;
int _drwr_hook_wpos_flip_vertical = SDL_FLIP_VERTICAL;
