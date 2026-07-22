#include "ett.h"
#include "log/log.h"

#include <assert.h>
#include <cglm/cglm.h>

EttMng ett_mng = {0};

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
    arena_destroy(&mng->arena);
}

Key ett_new(EttMng *mng, vec2 pos, mat2 aabb) {
    Key ett = poola_new(&mng->ett_pool);
    EttIns *ins = (EttIns *)poola_get(&mng->ett_pool, ett);

    glm_vec2_copy(pos, ins->pos);
    glm_mat2_copy(aabb, ins->aabb);

    b2AABB b2_aabb;
    b2_aabb.lowerBound.x = aabb[0][0];
    b2_aabb.lowerBound.y = aabb[0][1];
    b2_aabb.upperBound.x = aabb[1][0];
    b2_aabb.upperBound.y = aabb[1][1];

    b2DynamicTree_CreateProxy(&mng->aabb_tree, b2_aabb, 1, key2u64(ett));

    log_debug("Created a new entity %u:%u", ett.idx, ett.gen);

    return ett;
}

void ett_remv(EttMng *mng, Key ett) {
    if (!poola_alive(&mng->ett_pool, ett)) {
        log_err("ett_remv(): entity %u:%u is dead or invalid", ett.idx, ett.gen);
        return;
    }
    b2DynamicTree_DestroyProxy(&mng->aabb_tree, key2u64(ett));

    poola_remv(&mng->ett_pool, ett);

    log_debug("Removed an entity %u:%u", ett.idx, ett.gen);
}

EttIns *ett_get(EttMng *mng, Key ett) {
    if (!poola_alive(&mng->ett_pool, ett)) {
        log_err("ett_get(): entity %u:%u is dead or invalid => return stub", ett.idx, ett.gen);
        return (EttIns *)poola_get(&mng->ett_pool, (Key){0, 0});
    }
    return (EttIns *)poola_get(&mng->ett_pool, ett);
}
