#include "ett.h"
#include "log/log.h"

#include <assert.h>
#include <cglm/cglm.h>

EttMng ett_mng = {0};

void ett_mng_init(EttMng *mng, size_t etts_cap) {
    ++etts_cap; // for stub

    size_t arena_cap = poola_msize(sizeof(EttIns), etts_cap);
    arena_init(&mng->arena, arena_cap);

    poola_init(&mng->ett_pool, &mng->arena, sizeof(EttIns), alignof(EttIns), etts_cap);

    mng->aabb_tree = b2DynamicTree_Create();

    // stub
    size_t stub = poola_new(&mng->ett_pool);
    assert(stub == 0);
    b2DynamicTree_CreateProxy(&mng->aabb_tree, (b2AABB){0}, 0, (uint64_t)stub);
}

void ett_mng_destroy(EttMng *mng) {
    b2DynamicTree_Destroy(&mng->aabb_tree);

    arena_destroy(&mng->arena);
}

size_t ett_new(EttMng *mng, vec2 pos, mat2 aabb) {
    size_t ett = poola_new(&mng->ett_pool);
    EttIns *ins = (EttIns *)poola_get(&mng->ett_pool, ett);

    glm_vec2_copy(pos, ins->pos);
    glm_mat2_copy(aabb, ins->aabb);

    b2AABB b2_aabb;
    b2_aabb.lowerBound.x = aabb[0][0];
    b2_aabb.lowerBound.y = aabb[0][1];
    b2_aabb.upperBound.x = aabb[1][0];
    b2_aabb.upperBound.y = aabb[1][1];
    b2DynamicTree_CreateProxy(&mng->aabb_tree, b2_aabb, 1, ett);

    log_debug("Created a new entity %zu", ett);

    return ett;
}

void ett_remv(EttMng *mng, size_t ett) {
    if (!poola_alive(&mng->ett_pool, ett)) {
        log_warn("ett_remv(): entity %d is already removed or not valid");
        return;
    }
    poola_remv(&mng->ett_pool, ett);

    b2DynamicTree_DestroyProxy(&mng->aabb_tree, ett);

    log_debug("Removed an entity %d", ett);
}

EttIns *ett_get(EttMng *mng, size_t ett) {
    if (!poola_alive(&mng->ett_pool, ett)) {
        log_warn("ett_get(): entity %d is already removed or not valid => return stub");
        return (EttIns *)poola_get(&mng->ett_pool, 0);
    }
    return (EttIns *)poola_get(&mng->ett_pool, ett);
}
