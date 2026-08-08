#pragma once

#include "alloc/pool.h"
#include "math/vec.h"

typedef struct {
    Vec2 pos;
} Entity;

extern Pool entity_pool;

void entity_init(size_t cap);

u32 entity_new(Entity *data);

Entity *entity_ptr(u32 idx);
