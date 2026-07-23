#include "ett.h"
#include "log/log.h"

#include <assert.h>
#include <cglm/cglm.h>

void ett_mng_init(EttMng *mng, size_t etts_cap) {
    ++etts_cap; // for stub

    size_t arena_cap = pool_msize(sizeof(EttIns), etts_cap);
    arena_init(&mng->arena, arena_cap);

    poola_init(&mng->ett_pool, &mng->arena, sizeof(EttIns), alignof(EttIns), etts_cap);

    mng->aabb_tree = b2DynamicTree_Create();

    // stub
    Key stub = poola_new(&mng->ett_pool);
    assert(key2u64(stub) == 0);
    b2DynamicTree_CreateProxy(&mng->aabb_tree, (b2AABB){0}, 0, key2u64(stub));
}

void ett_mng_destroy(EttMng *mng) {
    b2DynamicTree_Destroy(&mng->aabb_tree);
    poola_destroy(&mng->ett_pool);
    arena_destroy(&mng->arena);
}

Key ett_new(EttMng *mng, vec2 pos,
            vec2 rect_offs, vec2 rect_centr, vec2 rect_size, uint64_t cat_bits) {
    assert(mng->arena.buffer);
    Key ett = poola_new(&mng->ett_pool);
    EttIns *ins = (EttIns *)poola_get(&mng->ett_pool, ett);

    glm_vec2_copy(pos, ins->pos);

    b2AABB aabb;
    aabb.lowerBound.x = pos[0] + rect_offs[0] - (rect_centr[0] * rect_size[0]);
    aabb.lowerBound.y = pos[1] + rect_offs[1] - (rect_centr[1] * rect_size[1]);
    aabb.upperBound.x = pos[0] + rect_offs[0] + ((1 - rect_centr[0]) * rect_size[0]);
    aabb.upperBound.y = pos[1] + rect_offs[1] + ((1 - rect_centr[1]) * rect_size[1]);

    b2DynamicTree_CreateProxy(&mng->aabb_tree, aabb, cat_bits, key2u64(ett));

    log_debug("Created a new entity %u.%u: pos: %.1f %.1f, aabb: %.1f:%.1f-%.1fx%.1f",
              ett.idx, ett.gen, pos[0], pos[1],
              aabb.lowerBound.x, aabb.lowerBound.y, aabb.upperBound.x, aabb.upperBound.y);

    return ett;
}

void ett_remv(EttMng *mng, Key ett) {
    assert(mng->arena.buffer);
    if (!poola_alive(&mng->ett_pool, ett)) {
        log_err("ett_remv(): entity %u.%u is dead or invalid", ett.idx, ett.gen);
        return;
    }
    b2DynamicTree_DestroyProxy(&mng->aabb_tree, key2u64(ett));

    poola_remv(&mng->ett_pool, ett);

    log_debug("Removed an entity %u.%u", ett.idx, ett.gen);
}

EttIns *ett_get(EttMng *mng, Key ett) {
    assert(mng->arena.buffer);
    if (!poola_alive(&mng->ett_pool, ett)) {
        log_err("ett_get(): entity %u.%u is dead or invalid => return stub", ett.idx, ett.gen);
        return (EttIns *)poola_get(&mng->ett_pool, (Key){0, 0});
    }
    return (EttIns *)poola_get(&mng->ett_pool, ett);
}
