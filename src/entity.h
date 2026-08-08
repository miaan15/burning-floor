#pragma once

#include "alloc/pool.h"
#include "math/vec.h"

typedef struct {
    u64 tag;
    Vec2 pos;
    Vec2 bounds;
} Entity;

extern Pool entity_pool;

void entity_init(size_t cap);

u32 entity_new(Entity *data);

Entity *entity_ptr(u32 idx);
