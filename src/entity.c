#include "entity.h"

#include "alloc/arena.h"
#include "context.h"
#include "log.h"
#include <stdalign.h>

Pool entity_pool = {0};

void entity_init(size_t cap) {
    pool_init_over(&entity_pool, arena_alloc(&global_ar, pool_caps(sizeof(Entity), cap), alignof(u64)), sizeof(Entity), cap);
}

u32 entity_new(Entity *data) {
    u32 entity = pool_new(&entity_pool, data);
    if (entity == (u32)-1) log_err("entity_new(): ");

    return entity;
}

Entity *entity_ptr(u32 idx) {
    return (Entity *)pool_ptr(&entity_pool, idx);
}
