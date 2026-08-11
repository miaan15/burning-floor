#include "action.h"

#include "context.h"
#include "entity.h"
#include "log.h"
#include <stdalign.h>

action *actions = NULL;
size_t actions_cap = 0;
size_t actions_len = 0;

void action_init(size_t cap) {
    actions = arena_alloc(&omni_arena, cap * sizeof(action), alignof(action));
    actions_cap = cap;
    actions_len = 0;
}

size_t action_new(u32 from, u32 to, u32 type, const void *arg_ptr) {
    action *ptr = &actions[actions_len];

    u32 arg; memcpy(&arg, arg_ptr, sizeof(u32));
    *ptr = (action){ from, to, type, arg };

    log_debug("Action %zu: from [%u] to [%u]: type = %u", actions_len, from, to, type);

    return actions_len++;
}

void action_update() {
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

    for (size_t i = 0; i < actions_len; ++i) {
        action a = actions[i];

        entity *to = entity_ptr(a.to);

        if (a.type == ACTION_HIT) {
            f32 damage; memcpy(&damage, &a.arg, sizeof(f32));
            to->health -= damage;
        }
    }

    actions_len = 0;
}
