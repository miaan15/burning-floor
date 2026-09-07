#include "effect.h"

#include "context.h"
#include <assert.h>

void effect_burn_make(effect *eff, u32 dur_ms, f32 damage, u32 interval_ms) {
    eff->type = EFF_BURN;
    memcpy(&eff->args[0], &damage, sizeof(f32));
    memcpy(&eff->args[1], &interval_ms, sizeof(u32));
    eff->args[3] = 0;
    eff->dur_ms = dur_ms;
    eff->passed_ms = 0;
}

void effect_burn_update(effect *eff, f32 *r_damage, bool *r_ended) {
    assert(eff->type == EFF_BURN);

    f32 damage; memcpy(&damage, &eff->args[0], sizeof(f32));
    u32 interval_ms; memcpy(&interval_ms, &eff->args[1], sizeof(u32));
    u32 lcheck_ms; memcpy(&lcheck_ms, &eff->args[2], sizeof(u32));

    *r_damage = 0;
    *r_ended = false;

    eff->passed_ms += tick_delta_ms;
    if (eff->passed_ms >= eff->dur_ms) {
        *r_ended = true;
        return;
    }

    if (eff->passed_ms - lcheck_ms > interval_ms) {
        *r_damage = damage;
    }

}
