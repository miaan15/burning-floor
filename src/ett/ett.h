#pragma once

#include "alloc/arena.h"
#include "common.h"
#include "pool/pool.h"
#include <box2d/collision.h>
#include <cglm/types.h>

typedef struct {
    uint64_t tag;
    vec2 pos;

    vec2 rect_offs;
    vec2 rect_centr;
    vec2 rect_size;
} EttIns;

typedef struct {
    Arena arena;
    PoolA ett_pool;
    b2DynamicTree aabb_tree;
} EttMng;

void ett_mng_init(EttMng *mng, size_t ett_cap);
void ett_mng_destroy(EttMng *mng);

Key ett_new(EttMng *mng, vec2 pos,
            vec2 rect_offs, vec2 rect_centr, vec2 rect_size, uint64_t cat_bits);
void ett_remv(EttMng *mng, Key ett);
EttIns *ett_get(EttMng *mng, Key ett);
