#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef char PoolMeta;

typedef struct {
    size_t esize;

    void *raw;
    size_t cap;

    size_t head;
    size_t maxi;
    size_t cnt;

    PoolMeta *meta;
} Pool;

typedef struct {
    void *ptr;
    PoolMeta meta;
} PoolResult;

size_t pool_req_size(size_t esize, size_t cap);

void pool_init(Pool *po, size_t esize, size_t cap);
void pool_init_over(Pool *po, void *root, size_t esize, size_t cap);

void pool_destroy(Pool *po);

PoolResult pool_new(Pool *po, void *data);

bool pool_remv(Pool *po, void *ptr, PoolMeta meta);
bool pool_remv_uc(Pool *po, void *ptr);

bool pool_alive(Pool *po, void *ptr, PoolMeta meta);

PoolMeta pool_meta(Pool *po, void *ptr);

void pool_reset(Pool *po);

#define pool_for(__pool, __ptr) \
    for (size_t __i = 0; __i < (__pool)->maxi; ++__i) \
        if ((__pool)->meta[__i] & 1) \
            for (void *__ptr = (char *)(__pool)->raw + (__i * (__pool)->esize), \
                 *__keep; __keep; __keep = 0)
