#include "entity.h"

#include "alloc/pool.h"
#include "context.h"
#include "log.h"
#include <stdalign.h>

pool entity_pool = {0};

void entity_init(size_t cap) {
    pool_init_in_arena(&entity_pool, &omni_arena, sizeof(entity), alignof(entity), cap);
}

size_t entity_new(entity *data) {
    size_t idx = pool_new(&entity_pool, data);

    log_info("Create Entity [%zu]", idx);

    return idx;
}

bool entity_remv(size_t idx) {
    if (!pool_remv(&entity_pool, idx)) {
        log_warn("entity_remv(): [%zu] is already dead", idx);
        return false;
    }

    log_info("Destroy Entity [%zu]", idx);

    return true;
}

entity *entity_ptr(size_t idx) {
    return (entity *)pool_ptr(&entity_pool, idx);
}

void _entity_add_effect(size_t idx, size_t eff) {
    entity *entity = pool_ptr(&entity_pool, idx);

    if (entity->effs) {
        size_t new_size = entity->effs[0] + 1;
        void *new_ptr = arena_alloc(tick_arena, (new_size + 1) * sizeof(u32), alignof(u32));
        memcpy(new_ptr, &entity->effs, entity->effs[0] * sizeof(u32));
        entity->effs = new_ptr;
        ++entity->effs[0];
        entity->effs[new_size] = eff;
    } else {
        entity->effs = arena_alloc(tick_arena, 2 * sizeof(u32), alignof(u32));
        memset(entity->effs, 0, 2 * sizeof(u32));
        ++entity->effs[0];
        entity->effs[1] = eff;
    }
}
