#include "effect.h"

#include "context.h"
#include "log.h"
#include <assert.h>

pool effect_pool = {0};

void effect_init(size_t cap) {
    pool_init_in_arena(&effect_pool, &omni_arena, sizeof(effect), cap * sizeof(effect), alignof(effect));

    // stub
    pool_new(&effect_pool, NULL);
}

size_t effect_new(effect *data) {
    size_t effect = pool_new(&effect_pool, data);
    if (effect == (size_t)-1) log_err("effect_new(): ");

    log_debug("Create effect [%zu]: type = %d", effect, data->type);

    return effect;
}

bool effect_remv(size_t idx) {
    if (!pool_alive(&effect_pool, idx)) {
        log_warn("effect_remv(): [idx] is already dead");
        return false;
    }
    effect *effect = pool_ptr(&effect_pool, idx);

    log_debug("Destroy effect [%zu]", idx);

    return true;
}

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
