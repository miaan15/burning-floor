#include "img.h"

#include "log/log.h"
#include <assert.h>

ImgMng img_mng = {0};
SprMng spr_mng = {0};
DrwrMng drwr_mng = {0};

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
        log_err("img_get(): image %d is out of bounds => return stub", img);
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

size_t spr_new(SprMng *mng, size_t img, SDL_FRect rect) {
    mng->raw[mng->len] = (SprIns){img, rect};
    log_debug("Made a new sprite %zu: img: %d, srect: %f:%f:%fx%f",
              mng->len, img, rect.x, rect.y, rect.w, rect.h);
    return mng->len++;
}

SprIns *spr_get(SprMng *mng, size_t spr) {
    if (spr >= mng->len) {
        log_err("spr_get(): sprite %d is out of bounds => return stub", spr);
        return mng->raw;
    }
    return &mng->raw[spr];
}

// =============================================================================
// DRAWER
void drwr_mng_init(DrwrMng *mng, size_t cap, SprMng *spr_mng, float pixel_scale) {
    ++cap; // stub

    size_t arena_size = 0;
    arena_size += poola_msize(sizeof(DrwrIns), cap);
    arena_size = align_up(arena_size, alignof(DrwrHook));
    arena_size += poola_msize(sizeof(DrwrHook), cap);
    arena_init(&mng->arena, arena_size);

    poola_init(&mng->drwr_pool, &mng->arena, sizeof(DrwrIns), alignof(DrwrIns), cap);
    poola_init(&mng->hook_pool, &mng->arena, sizeof(DrwrHook), alignof(DrwrHook), cap);

    mng->spr_mng = spr_mng;

    mng->pixel_scale = pixel_scale;

    // stub
    size_t drwr_stub = poola_new(&mng->drwr_pool);
    *(DrwrIns *)poola_get(&mng->drwr_pool, drwr_stub) = (DrwrIns){0}; // FIXME
    size_t hook_stub = poola_new(&mng->hook_pool);
    *(DrwrHook *)poola_get(&mng->hook_pool, hook_stub) = (DrwrHook){0}; // FIXME
}

void drwr_mng_destroy(DrwrMng *mng) {
    arena_destroy(&mng->arena);
}

void _drwr_handle_hook_wpos_update(DrwrMng *mng,
                                   DrwrIns *drwr_ins, DrwrHook *hook_ins);
void drwr_mng_update(DrwrMng *mng) {
    // FIXME pool support iterate plz
    for (size_t i = 0; i < mng->drwr_pool.offset; ++i) {
        size_t drwr = (size_t)i;
        if (!poola_alive(&mng->drwr_pool, drwr)) continue;

        DrwrIns *drwr_ins = (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
        DrwrHook *hook_ins = (DrwrHook *)poola_get(&mng->hook_pool, drwr);

        switch (hook_ins->type) {
        case DRWR_HOOK_WPOS:
            _drwr_handle_hook_wpos_update(mng, drwr_ins, hook_ins);
        break;
        case DRWR_HOOK_NONE: break;
        default: log_err("drwr_mng_update(), drwr %d's hook data is smt wrong idk", drwr); break;
        }
    }
}

void drwr_mng_draw(DrwrMng *mng, SDL_Renderer *renderer, SDL_Window *window) {
    int window_w, window_h;
    SDL_GetWindowSize(window, &window_w, &window_h);

    SprMng *_spr_mng = mng->spr_mng;
    ImgMng *_img_mng = _spr_mng->img_mng;

    // FIXME pool support iterate plz
    for (size_t i = 0; i < mng->drwr_pool.offset; ++i) {
        size_t drwr = (size_t)i;
        if (!poola_alive(&mng->drwr_pool, drwr)) continue;

        DrwrIns *ins = (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
        assert((size_t)ins->spr < _spr_mng->len);
        SprIns spr_ins = _spr_mng->raw[(size_t)ins->spr];
        assert((size_t)spr_ins.img < _img_mng->len);
        ImgIns img_ins = _img_mng->raw[(size_t)spr_ins.img];

        SDL_FRect drect = ins->drect;
        drect.y = window_h - drect.y;

        SDL_RenderTextureRotated(renderer, img_ins.tex, &spr_ins.rect, &drect,
                                 0, NULL, ins->flip);
    }
}

size_t drwr_new(DrwrMng *mng, size_t spr, size_t z_lv) {
    size_t drwr = poola_new(&mng->drwr_pool);
    size_t hook = poola_new(&mng->hook_pool);
    assert(drwr == hook);
    if (drwr < 0) {
        log_err("drwr_new(): cannot create new drawer => return stub");
        return 0;
    }
    DrwrIns *drwr_ins = (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
    drwr_ins->spr = spr;
    drwr_ins->z_lv = z_lv;
    drwr_ins->active = true;
    // DrwrHook *hook_ins = (DrwrHook *)poola_get(&mng->hook_pool, hook);
    return drwr;
}

void drwr_remv(DrwrMng *mng, size_t drwr) {
    if (!poola_alive(&mng->drwr_pool, drwr)) {
        log_err("drwr_remv(): drawer %d is dead or invalid", drwr);
        return;
    }
    poola_remv(&mng->drwr_pool, drwr);
    poola_remv(&mng->hook_pool, drwr);
}

void drwr_set_draw(DrwrMng *mng, size_t drwr, bool active) {
    if (!poola_alive(&mng->drwr_pool, drwr)) {
        log_err("drwr_set_draw(): drawer %d is dead or invalid", drwr);
    }
    DrwrIns *ins = (DrwrIns *)poola_get(&mng->drwr_pool, drwr);
    ins->active = active;
}

void drwr_hook_wpos(DrwrMng *mng, size_t drwr,
                    float *pos, float *offset, float *center, bool *flip, float *scale) {
    if (!poola_alive(&mng->hook_pool, drwr)) {
        log_err("drwr_hook_wpos(): drawer %d is dead or invalid", drwr);
    }
    DrwrHook *hook_ins = (DrwrHook *)poola_get(&mng->hook_pool, drwr);
    hook_ins->type = DRWR_HOOK_WPOS;
    hook_ins->wpos_pos = pos;
    hook_ins->wpos_offset = offset;
    hook_ins->wpos_center = center;
    hook_ins->wpos_flip = flip;
    hook_ins->wpos_scale = scale;
}

const float _drwr_hook_wpos_pos_zero[2] = {0, 0};
const float _drwr_hook_wpos_offset_zero[2] = {0, 0};
const float _drwr_hook_wpos_center_mid[2] = {.5, .5};
const float _drwr_hook_wpos_center_bl[2] = {0, 0};
const bool _drwr_hook_wpos_flip_no = false;
const bool _drwr_hook_wpos_flip_yes = true;
const float _drwr_hook_wpos_scale_one[2] = {1, 1};

void _drwr_handle_hook_wpos_update(DrwrMng *mng,
                                   DrwrIns *drwr_ins, DrwrHook *hook_ins) {
    assert(hook_ins->type == DRWR_HOOK_WPOS);
    vec2 _pos = {hook_ins->wpos_pos[0] + hook_ins->wpos_offset[0],
                 hook_ins->wpos_pos[1] + hook_ins->wpos_offset[1]};
    SprIns *spr_ins = spr_get(mng->spr_mng, drwr_ins->spr);
    vec2 _size = {spr_ins->rect.x * mng->pixel_scale * hook_ins->wpos_scale[0],
                  spr_ins->rect.y * mng->pixel_scale * hook_ins->wpos_scale[1]};
    drwr_ins->drect.x = _pos[0] - (hook_ins->wpos_center[0] * _size[0]);
    drwr_ins->drect.y = _pos[1] - (hook_ins->wpos_center[1] * _size[1]);
    drwr_ins->drect.w = _size[0];
    drwr_ins->drect.h = _size[1];
    drwr_ins->flip = *hook_ins->wpos_flip;
}
