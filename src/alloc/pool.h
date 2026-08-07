#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t esize;

    void *raw;
    size_t cap;

    size_t head;
    size_t maxi;
    size_t cnt;

    size_t *meta;
} Pool;

size_t pool_caps(size_t esize, size_t cap);

void pool_init(Pool *po, size_t esize, size_t cap);
void pool_init_over(Pool *po, void *root, size_t esize, size_t cap);

void pool_destroy(Pool *po);

size_t pool_new(Pool *po, void *data);

bool pool_remv(Pool *po, size_t idx);
void pool_remv_uc(Pool *po, size_t idx);

bool pool_alive(Pool *po, size_t idx);

void *pool_ptr(Pool *po, size_t idx);
size_t pool_index(Pool *po, void *ptr);

void pool_reset(Pool *po);

#define pool_for(__pool, __idx, __ptr) \
    for (size_t __i = 0; __i < (__pool)->maxi; ++__i) \
        if ((__pool)->meta[__i] == (size_t)-1 && \
            ((__idx = __i)) && \
            ((__ptr) = (char *)(__pool)->raw + (__i * (__pool)->esize)))
