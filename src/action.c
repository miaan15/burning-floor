#include "action.h"

#include "context.h"
#include "effect.h"
#include "entity.h"
#include "log.h"
#include <assert.h>
#include <stdalign.h>

action *actions = NULL;
size_t actions_cap = 0;
size_t actions_len = 0;

void action_init(size_t cap) {
    actions = arena_alloc(&omni_arena, cap * sizeof(action), alignof(action));
    actions_cap = cap;
    actions_len = 0;
}

size_t action_new(u32 from, u32 to, u32 type, const void *arg) {
    action *ptr = &actions[actions_len];

    prx32 _arg;
    if (arg) memcpy(&_arg, arg, sizeof(prx32));
    else arg = 0;

    *ptr = (action){ from, to, type, _arg };

    log_debug("Action %zu: from [%u] to [%u]: type = %u", actions_len, from, to, type);

    return actions_len++;
}

void handle_actions_effect(size_t begin, size_t end, u32 to) {
    log_info("handle %zu - %zu of %u", begin, end, to);
    entity *entity = entity_ptr(to);

    // copy old entity->effs
    size_t len = (end - begin) + (entity->effs != NULL ? *entity->effs : 0);

    size_t offs = sizeof(u32) + (entity->effs != NULL ? *entity->effs * sizeof(effect) : 0);

    u32 *new_ptr = arena_alloc(tick_arena, sizeof(u32) + len * sizeof(effect), alignof(effect));
    if (entity->effs) memcpy(new_ptr + 1, entity->effs + 1, *entity->effs * sizeof(effect));
    entity->effs = new_ptr;

    *entity->effs = len;

    for (size_t i = begin; i < end; ++i) {
        u32 type = actions[i].type;
        prx32 arg = actions[i].arg;

        assert(actions[i].to == to);
        assert(type >= ACTION_EFF_OFFS);

        switch (type) {
        case EFF_BURN: {
            f32 damage; memcpy(&damage, &arg, sizeof(f32));
            effect_burn_make((effect *)((char *)entity->effs + offs), 1000, damage, 250);
            } break;
        }

        offs += sizeof(effect);
    }
}

void action_update() {
    // sort all actions by [to:type]
    // insertion sort
    for (size_t i = 1; i < actions_len; ++i) {
        action a = actions[i];
        u64 k = ((u64)a.to << 32) | (u64)a.type;
        size_t j = i;

        while (j > 0 && (((u64)actions[j - 1].to << 32) | (u64)actions[j - 1].type) > k) {
            actions[j] = actions[j - 1];
            --j;
        }

        actions[j] = a;
    }

    u32 last_to = actions[0].to;
    size_t last_not_eff = 0;
    for (size_t i = 0; i < actions_len; ++i) {
        action a = actions[i];
        entity *e_to = entity_ptr(a.to);

        if (a.type == ACTION_HIT) {
            f32 damage; memcpy(&damage, &a.arg, sizeof(f32));
            e_to->health -= damage;
        }

        if (a.type < ACTION_EFF_OFFS) { last_not_eff = i + 1; }

        if (a.to != last_to) {
            handle_actions_effect(last_not_eff, i, last_to);
        }

        if (i == actions_len - 1) {
            handle_actions_effect(last_not_eff, actions_len, a.to);
        }

        last_to = a.to;
    }

    actions_len = 0;
}
