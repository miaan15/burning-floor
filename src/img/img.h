#pragma once

#include "macro.h"
#include "pool/pool.h"
#include <cglm/types.h>
#include <SDL3/SDL_render.h>

// =============================================================================
// IMAGE
typedef struct {
    SDL_Texture *tex;
    i32 w, h;
} ImgIns;

typedef struct {
    ImgIns *raw;
    size_t cap;
    size_t len;
} ImgMng;

extern ImgMng img_mng;

void img_mng_init(ImgMng *mng, size_t cap);
void img_mng_destroy(ImgMng *mng);

i32 img_new(ImgMng *mng, const char *path,
            SDL_Renderer *renderer, SDL_ScaleMode scalemode);

// =============================================================================
// SPRITE
typedef struct {
    i32 img;
    SDL_FRect rect;
} SprIns;

typedef struct {
    SprIns *raw;
    size_t cap;
    size_t len;

    ImgMng *img_mng;
} SprMng;

extern SprMng spr_mng;

void spr_mng_init(SprMng *mng, size_t cap, ImgMng *img_mng);
void spr_mng_destroy(SprMng *mng);

i32 spr_new(SprMng *mng, i32 img, SDL_FRect rect);
SprIns *spr_get(SprMng *mng, i32 spr);

// =============================================================================
// DRAWER
enum DrwrHookType {
    DRWR_HOOK_NONE = 0,
    DRWR_HOOK_WPOS
};
typedef struct {
    i32 type;
    union {
        struct {
            f32 *wpos_pos;
            f32 *wpos_offset;
            f32 *wpos_center;
            bool *wpos_flip;
            f32 *wpos_scale;
        };
    };
} DrwrHook;

typedef struct {
    i32 spr;

    SDL_FRect drect;
    SDL_FlipMode flip;

    bool active;
    i32 z_lv;
} DrwrIns;

typedef struct {
    Arena arena;
    PoolA drwr_pool;
    PoolA hook_pool;

    SprMng *spr_mng;

    f32 pixel_scale;
} DrwrMng;

extern DrwrMng drwr_mng;

void drwr_mng_init(DrwrMng *mng, size_t cap, SprMng *SprMng, f32 pixel_scale);
void drwr_mng_destroy(DrwrMng *mng);
void drwr_mng_update(DrwrMng *mng);
void drwr_mng_draw(DrwrMng *mng, SDL_Renderer *renderer, SDL_Window *window);

i32 drwr_new(DrwrMng *mng, i32 spr, i32 z_lv);
void drwr_remv(DrwrMng *mng, i32 drwr);

void drwr_set_draw(DrwrMng *mng, i32 drwr, bool active);

void drwr_hook_wpos(DrwrMng *mng, i32 drwr,
                    f32 *pos, f32 *offset, f32 *center, bool *flip, f32 *scale);

extern const f32 _drwr_hook_wpos_pos_zero[2];
extern const f32 _drwr_hook_wpos_offset_zero[2];
extern const f32 _drwr_hook_wpos_center_mid[2];
extern const f32 _drwr_hook_wpos_center_bl[2];
extern const bool _drwr_hook_wpos_flip_no;
extern const bool _drwr_hook_wpos_flip_yes;
extern const f32 _drwr_hook_wpos_scale_one[2];

#define DRWR_HOOK_WPOS_POS_ZERO (_drwr_hook_wpos_pos_zero)
#define DRWR_HOOK_WPOS_OFFSET_ZERO (_drwr_hook_wpos_offset_zero)
#define DRWR_HOOK_WPOS_CENTER_MID (_drwr_hook_wpos_center_mid)
#define DRWR_HOOK_WPOS_CENTER_BL (_drwr_hook_wpos_center_bl)
#define DRWR_HOOK_WPOS_FLIP_NO (&_drwr_hook_wpos_flip_no)
#define DRWR_HOOK_WPOS_FLIP_YES (&_drwr_hook_wpos_flip_yes)
#define DRWR_HOOK_WPOS_SCALE_ONE (_drwr_hook_wpos_scale_one)
