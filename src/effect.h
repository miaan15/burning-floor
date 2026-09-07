#pragma once

#include "macro.h"
#include <stdbool.h>
#include <stdalign.h>

typedef struct {
    i32 type;
    u32 args[3];
    u32 dur_ms;
    u32 passed_ms;
} effect;

_Static_assert(alignof(effect) == alignof(u32), "effect alignment should = 4");

typedef enum {
    EFF_NONE,
    EFF_BURN,
    EFF_KNOCK
} effect_type;

void effect_burn_make(effect *eff, u32 dur_ms, f32 damage, u32 interval_ms);
void effect_burn_update(effect *eff, f32 *r_damage, bool *r_ended);
