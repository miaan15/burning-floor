#pragma once

#include "allocator/allocator.h"
#include "macro.h"
#include "pool/pool.h"
#include <box2d/collision.h>
#include <cglm/types.h>

typedef struct {
    u64 tag;
    vec2 pos;
    mat2 aabb;
} EttIns;

typedef struct {
    Arena arena;
    PoolA ett_pool;
    b2DynamicTree aabb_tree;
} EttMng;

extern EttMng ett_mng;

void ett_mng_init(EttMng *mng, size_t ett_cap);
void ett_mng_destroy(EttMng *mng);

i32 ett_new(EttMng *mng, vec2 pos, mat2 aabb);
void ett_remv(EttMng *mng, i32 ett);
EttIns *ett_get(EttMng *mng, i32 ett);
