#include "entity.h"

#include "alloc/pool.h"
#include "context.h"
#include "log.h"
#include <stdalign.h>

pool entity_pool = {0};

void entity_init(size_t cap) {
    pool_init_in_arena(&entity_pool, &global_ar, sizeof(entity), alignof(entity), cap);
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
