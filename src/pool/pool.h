#pragma once

#include "allocator/allocator.h"

typedef int32_t i32;

typedef struct {
    Arena *arena;

    size_t esize;
    void *data;
    size_t cap;
    size_t cnt;
    size_t offset;
    i32 next;
    void *alive;
} Pool;

static inline size_t pool_msize(size_t esize, size_t cap) {
    return (esize * cap) + ((cap + 7) / 8);
}

void pool_init(Pool *pool, Arena *arena, size_t esize, size_t cap);

i32 pool_add(Pool *pool);

void pool_remv(Pool *pool, i32 obj);

void pool_reset(Pool *pool);

void *pool_get(Pool *pool, i32 obj);
