#include "effect.h"

#include "context.h"
#include <assert.h>

void effect_burn_apply(effect *eff, f32 *r_damage, bool *r_end) {
    assert(eff->type == EFF_BURN);
    f32 damage = eff->args[0];
    f32 interval = eff->args[1];
    f32 last_check = eff->args[2];

    *r_damage = 0;
    *r_end = false;

    eff->passed_ms += tick_delta_ms;
    if (eff->passed_ms >= eff->dur_ms) {
        *r_end = true;
        return;
    }

    if (eff->passed_ms - last_check > interval) {
        *r_damage = damage;
    }
}

void effect_knock_apply(effect *eff, vec2 *r_from, f32 *r_mag) {
    assert(eff->type == EFF_BURN);
    vec2 from = { eff->args[0], eff->args[1] };
    f32 mag = eff->args[1];

    *r_from = from;
    *r_mag = mag;
}
