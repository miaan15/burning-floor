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
    mat2 srect;
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

enum DrwrHookType {
    DRWR_HOOK_NONE = 0,
    DRWR_HOOK_WPOS,
    DRWR_HOOK_SWPOS
};

typedef struct {
    int type;
    union {
        struct {
            float *wpos_pos;
            float *wpos_offs;
            float *wpos_centr;
            float *wpos_rot;
            int *wpos_flip;
            float *wpos_scale;
        };

        struct {
            float *swpos_pos;
            float *swpos_rot;
            float *swpos_scale;
        };
    };
} DrwrHook;

typedef struct {
    size_t spr;
    vec2 srect_size;

    mat2 drect;
    int flip;
    float rot;
    vec2 centr;

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

Key drwr_new(DrwrMng *mng, size_t spr, int z);
void drwr_remv(DrwrMng *mng, Key drwr);

DrwrIns *drwr_get(DrwrMng *mng, Key drwr);
DrwrHook *drwr_get_hook(DrwrMng *mng, Key drwr);

static inline void drwr_set_spr(DrwrMng *mng, Key drwr, size_t spr) {
    DrwrIns *drwr_ins = drwr_get(mng, drwr);
    SprIns *spr_ins = spr_get(mng->spr_mng, spr);
    drwr_ins->spr = spr;
    drwr_ins->srect_size[0] = spr_ins->srect[1][0];
    drwr_ins->srect_size[1] = spr_ins->srect[1][1];
}

static inline void drwr_set_active(DrwrMng *mng, Key drwr, bool active) {
    drwr_get(mng, drwr)->active = active;
}

static inline void drwr_set_z(DrwrMng *mng, Key drwr, int z) {
    drwr_get(mng, drwr)->z = z;
}

static inline void drwr_hook_disable(DrwrMng *mng, Key drwr) {
    memset(drwr_get_hook(mng, drwr), 0, sizeof(DrwrHook));
}

void drwr_hook_set_wpos(DrwrMng *mng, Key drwr,
                        float *pos, float *offs, float *centr,
                        float *rot, int *flip, float *scale);
void drwr_feed_wpos(DrwrIns *drwr_ins,
                    vec2 pos, vec2 offs, vec2 centr,
                    vec2 rot, int *flip, vec2 scale);

void drwr_hook_set_swpos(DrwrMng *mng, Key drwr, float *pos, float *rot, float *scale);
void drwr_feed_swpos(DrwrIns *drwr_ins, float *pos, float *rot, float *scale);

extern int _drwr_hook_wpos_flip_horizontal;
#define DRWR_HOOK_WPOS_FLIP_HORIZONTAL (&_drwr_hook_wpos_flip_horizontal)
extern int _drwr_hook_wpos_flip_vertical;
#define DRWR_HOOK_WPOS_FLIP_VERTICAL (&_drwr_hook_wpos_flip_vertical)
