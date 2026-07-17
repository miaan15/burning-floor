#pragma once

#include "alloc/arena.h"
#include "pool/pool.h"
#include <box2d/collision.h>
#include <cglm/types.h>

typedef struct {
    uint64_t tag;
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

size_t ett_new(EttMng *mng, vec2 pos, mat2 aabb);
void ett_remv(EttMng *mng, size_t ett);
EttIns *ett_get(EttMng *mng, size_t ett);
