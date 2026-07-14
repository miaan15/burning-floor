#include "entity.h"
#include "log/log.h"
#include <stdalign.h>

EttMng ett_mng = {0};

b2DynamicTree _broad_aabb_tree = {0};

void _ett_set_deleted(EttMng *mng, size_t index, bool v) {
    char *bitset = (char *)(mng->etts_deleted);
    size_t byte_idx = index / 8;
    size_t bit_idx = index % 8;
    bitset[byte_idx] = (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
}
bool _ett_is_deleted(EttMng *mng, size_t index) {
    char *bitset = (char *)(mng->etts_deleted);
    return (bitset[index / 8] >> (index % 8)) & 1;
}

void ett_mng_init(EttMng *mng, size_t etts_cap) {
    ++etts_cap; // for stub
    size_t arena_cap = 0;
    arena_cap += etts_cap * sizeof(EttIns);
    arena_cap += (etts_cap + 7) / 8;
    arena_init(&mng->arena, arena_cap);

    mng->etts_cap = etts_cap;
    mng->etts = (EttIns *)arena_alloc(&mng->arena,
        etts_cap * sizeof(EttIns), alignof(EttIns));
    mng->etts_deleted = arena_alloc(&mng->arena, (etts_cap + 7) / 8, 1);
    mng->etts_offset = mng->etts_count = mng->etts_next_id = 0;

    mng->aabb_tree = b2DynamicTree_Create();

    // stub
    ++mng->etts_offset; ++mng->etts_count; ++mng->etts_next_id;
    mng->etts[0] = (EttIns){0};
    b2DynamicTree_CreateProxy(&mng->aabb_tree, (b2AABB){0}, 0, 0);
}

void ett_mng_destroy(EttMng *mng) {
    b2DynamicTree_Destroy(&mng->aabb_tree);
    arena_destroy(&mng->arena);
}

size_t ett_make(EttMng *mng, b2Vec2 pos, b2AABB aabb) {
    if (mng->etts_count >= mng->etts_cap) {
        log_err("ett_make(): full entity capacity => Return 0");
        return 0;
    }

    size_t ett = mng->etts_next_id;
    if (ett == mng->etts_offset) {
        ++mng->etts_offset;
        ++mng->etts_next_id;
    } else {
        memcpy(&mng->etts_next_id, &mng->etts[ett], sizeof(size_t));
    }

    _ett_set_deleted(mng, ett, 0);
    ++mng->etts_count;

    memset(&mng->etts[ett], 0, sizeof(EttIns));

    mng->etts[ett].pos = pos;
    mng->etts[ett].aabb = aabb;
    b2DynamicTree_CreateProxy(&mng->aabb_tree, aabb, 1, ett);

    log_debug("Made an entity %zu", ett);

    return ett;
}

void ett_remv(EttMng *mng, size_t ett) {
    if (ett >= mng->etts_offset) {
        log_err("ett_remv(): index %zu out of bounds", ett);
        return;
    }
    if (_ett_is_deleted(mng, ett)) {
        log_err("ett_remv(): ett %zu already removed", ett);
        return;
    }

    memcpy(&mng->etts[ett], &mng->etts_next_id, sizeof(size_t));
    mng->etts_next_id = ett;

    _ett_set_deleted(mng, ett, 1);
    --mng->etts_count;

    b2DynamicTree_DestroyProxy(&mng->aabb_tree, ett);

    log_debug("Removed an ett %zu", ett);
}

EttIns *ett_get(EttMng *mng, size_t ett) {
    if (ett >= mng->etts_offset) {
        log_err("ett_remv(): index %zu out of bounds => Return stub", ett);
        return &mng->etts[0];
    }
    if (_ett_is_deleted(mng, ett)) {
        log_err("ett_remv(): ett %zu already removed => Return stub", ett);
        return &mng->etts[0];
    }
    return &mng->etts[ett];
}
