#pragma once

#include "allocator/allocator.h"

typedef int32_t i32;

typedef struct {
    Arena arena;

    size_t esize;
    void *data;
    size_t cap;
    size_t cnt;
    size_t offset;
    i32 next;
    void *alive;
} Pool;

void pool_init(Pool *pool, size_t esize, size_t cap);
void pool_destroy(Pool *pool);

i32 pool_add(Pool *pool);

void pool_remv(Pool *pool, i32 obj);

void pool_reset(Pool *pool);
