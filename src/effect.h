#pragma once

#include "alloc/pool.h"
#include "macro.h"
#include "math/vec.h"
#include <stdbool.h>

typedef struct {
    i32 type;
    i32 cat; // TODO continuous, instant,...

    f32 args[3];

    u32 dur_ms;
    u32 passed_ms;
} effect;

typedef enum {
    EFF_NONE,
    EFF_BURN,
    EFF_KNOCK
} effect_type;

typedef enum {
    EFF_CAT_INS,
    EFF_CAT_DUR
} effect_cat;

extern pool effect_pool;

void effect_init(size_t cap);

size_t effect_new(effect *data);

bool effect_remv(size_t idx);

void effect_burn_apply(effect *eff, f32 *r_damage, bool *r_end);

void effect_knock_apply(effect *eff, vec2 *r_from, f32 *r_mag);
