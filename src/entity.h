#pragma once

#include "alloc/pool.h"
#include "math/vec.h"

typedef struct {
    u64 tag;
    vec2 pos;
    vec2 bounds;
} entity;

extern pool entity_pool;

void entity_init(size_t cap);

size_t entity_new(entity *data);

entity *entity_ptr(size_t idx);
