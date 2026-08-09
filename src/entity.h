#pragma once

#include "alloc/pool.h"
#include "math/vec.h"

typedef struct {
    u64 tag;
    vec2 pos;
    vec2 bounds;
    float health;
} entity;

extern pool entity_pool;

void entity_init(size_t cap);

size_t entity_new(entity *data);

bool entity_remv(size_t idx);

entity *entity_ptr(size_t idx);
