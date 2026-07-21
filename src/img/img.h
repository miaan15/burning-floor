#pragma once

#include "pool/pool.h"
#include <cglm/types.h>
#include <SDL3/SDL_render.h>

// =============================================================================
// IMAGE
typedef struct {
    SDL_Texture *tex;
    size_t w, h;
} ImgIns;

typedef struct {
    ImgIns *raw;
    size_t cap;
    size_t len;
} ImgMng;

void img_mng_init(ImgMng *mng, size_t cap);
void img_mng_destroy(ImgMng *mng);

size_t img_new(ImgMng *mng, const char *path,
               SDL_Renderer *renderer, SDL_ScaleMode scalemode);

ImgIns *img_get(ImgMng *mng, size_t img);

// =============================================================================
// SPRITE
typedef struct {
    size_t img;
    mat2 rect;
} SprIns;

typedef struct {
    SprIns *raw;
    size_t cap;
    size_t len;

    ImgMng *img_mng;
} SprMng;

void spr_mng_init(SprMng *mng, size_t cap, ImgMng *img_mng);
void spr_mng_destroy(SprMng *mng);

size_t spr_new(SprMng *mng, size_t img, mat2 rect);
SprIns *spr_get(SprMng *mng, size_t spr);

// =============================================================================
// DRAWER
typedef enum {
    DRWR_HOOK_NONE = 0,
    DRWR_HOOK_WPOS
} DrwrHookType;

typedef struct {
    DrwrHookType type;
    union {
        struct {
            float *wpos_pos;
            float *wpos_offset;
            float *wpos_center;
            float *wpos_rot;
            int *wpos_flip;
            float *wpos_scale;
        };
    };
} DrwrHook;

typedef struct {
    size_t spr;

    mat2 drect;
    int flip;
    float rot;
    vec2 center;

    int z;

    bool active;
} DrwrIns;

typedef struct {
    Arena arena;
    PoolA drwr_pool;
    PoolA hook_pool;

    SprMng *spr_mng;

    float scale;
    int pixel_size;
} DrwrMng;

void drwr_mng_init(DrwrMng *mng, size_t cap, SprMng *spr_mng, float scale, int pixel_size);
void drwr_mng_destroy(DrwrMng *mng);
void drwr_mng_update(DrwrMng *mng);
void drwr_mng_draw(DrwrMng *mng, SDL_Renderer *renderer, SDL_Window *window);

size_t drwr_new(DrwrMng *mng, size_t spr, size_t z_lv);
void drwr_remv(DrwrMng *mng, size_t drwr);

DrwrIns *drwr_get(DrwrMng *mng, size_t drwr);
DrwrHook *drwr_get_hook(DrwrMng *mng, size_t drwr);

static inline void drwr_set_spr(DrwrMng *mng, size_t drwr, size_t spr) {
    drwr_get(mng, drwr)->spr = spr;
}

static inline void drwr_set_active(DrwrMng *mng, size_t drwr, bool active) {
    drwr_get(mng, drwr)->active = active;
}

static inline void drwr_set_z(DrwrMng *mng, size_t drwr, int z) {
    drwr_get(mng, drwr)->z = z;
}

static inline void drwr_hook_disable(DrwrMng *mng, size_t drwr) {
    memset(drwr_get_hook(mng, drwr), 0, sizeof(DrwrHook));
}

static inline void drwr_hook_set_wpos(DrwrMng *mng, size_t drwr,
                                      float *pos, float *offset, float *center,
                                      float *rot, int *flip, float *scale) {
    DrwrHook *hook = drwr_get_hook(mng, drwr);
    hook->type = DRWR_HOOK_WPOS;
    hook->wpos_pos = pos;
    hook->wpos_offset = offset;
    hook->wpos_center = center;
    hook->wpos_rot = rot;
    hook->wpos_flip = flip;
    hook->wpos_scale = scale;
}

void drwr_feed_wpos(DrwrMng *mng, size_t drwr,
                    vec2 pos, vec2 offset, vec2 center,
                    vec2 rot, int *flip, vec2 scale);

extern float _drwr_hook_wpos_center_mid[2];
#define DRWR_HOOK_WPOS_CENTER_MID (_drwr_hook_wpos_center_mid)
extern int _drwr_hook_wpos_flip_horizontal;
#define DRWR_HOOK_WPOS_FLIP_HORIZONTAL (&_drwr_hook_wpos_flip_horizontal)
extern int _drwr_hook_wpos_flip_vertical;
#define DRWR_HOOK_WPOS_FLIP_VERTICAL (&_drwr_hook_wpos_flip_vertical)
