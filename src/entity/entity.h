#pragma once

#include "allocator/allocator.h"
#include <stddef.h>
#include <stdint.h>
#include <box2d/collision.h>

typedef struct {
    uint64_t tag;
    b2Vec2 pos;
    b2AABB aabb;
} EttIns;

typedef struct {
    Arena arena;

    EttIns *etts;
    size_t etts_offset;
    size_t etts_count;
    size_t etts_cap;
    size_t etts_next_id;
    void *etts_deleted;

    b2DynamicTree aabb_tree;
} EttMng;

extern EttMng ett_mng;

void ett_mng_init(EttMng *mng, size_t ett_cap);
void ett_mng_destroy(EttMng *mng);

size_t ett_make(EttMng *mng, b2Vec2 pos, b2AABB aabb);
void ett_remv(EttMng *mng, size_t ett);
EttIns *ett_get(EttMng *mng, size_t ett);
