#pragma once

#include "alloc/arena.h"
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>

typedef struct {
    size_t esize;
    size_t ealign;

    void *raw;
    size_t cap;
    size_t offset;
    size_t cnt;

    size_t head;
    size_t *next;
} Pool;

void pool_init(Pool *pool, size_t esize, size_t ealign, size_t cap);
static inline Pool pool_make(size_t esize, size_t ealign, size_t cap) {
    Pool p; pool_init(&p, esize, ealign, cap); return p;
}
void pool_destroy(Pool *pool);

size_t pool_new(Pool *pool);
bool pool_remv(Pool *pool, size_t obj);
void pool_reset(Pool *pool);
bool pool_alive(Pool *pool, size_t obj);
void *pool_get(Pool *pool, size_t obj);

static inline size_t pool_msize(size_t esize, size_t cap) {
    size_t size = 0;
    size += cap * esize;
    size = align_up(size, alignof(size_t));
    size += cap * sizeof(size_t);
    return size;
}
static inline size_t pool_malign(size_t ealign) {
    size_t salign = alignof(size_t);
    return ealign > salign ? ealign : salign;
}

#define pool_for(pool_ptr, idx) \
    for (size_t idx = 0; idx < (pool_ptr)->offset; ++idx) \
        if ((pool_ptr)->next[idx] == _ALIVE_VAL)

typedef struct {
    Pool *pool;
    size_t idx;
    void *data;
} PoolIt;

static inline PoolIt pool_iter(Pool *pool) {
    return (PoolIt){ pool, (size_t)-1, NULL };
}

static inline bool pool_next(PoolIt *it) {
    Pool *pool = it->pool;
    assert(pool && pool->raw);

    while (++it->idx < pool->offset) {
        if (pool->next[it->idx] == (size_t)-1) {
            it->data = (char *)pool->raw + (it->idx * pool->esize);
            return true;
        }
    }

    it->data = NULL;
    return false;
}

// =============================================================================
typedef struct {
    Arena *arena;

    size_t esize;
    size_t ealign;

    void *raw;
    size_t cap;
    size_t offset;
    size_t cnt;

    size_t head;
    size_t *next;
} PoolA;

void poola_init(PoolA *pool, Arena *arena, size_t esize, size_t ealign, size_t cap);
static inline PoolA poola_make(Arena *arena, size_t esize, size_t ealign, size_t cap) {
    PoolA p; poola_init(&p, arena, esize, ealign, cap); return p;
}
void poola_destroy(PoolA *pool);

size_t poola_new(PoolA *pool);
bool poola_remv(PoolA *pool, size_t obj);
void poola_reset(PoolA *pool);
bool poola_alive(PoolA *pool, size_t obj);
void *poola_get(PoolA *pool, size_t obj);

static inline size_t poola_msize(size_t esize, size_t cap) {
    size_t size = 0;
    size += cap * esize;
    size = align_up(size, alignof(size_t));
    size += cap * sizeof(size_t);
    return size;
}
static inline size_t poola_malign(size_t ealign) {
    size_t salign = alignof(size_t);
    return ealign > salign ? ealign : salign;
}

#define poola_for(poola_ptr, idx) \
    for (size_t idx = 0; idx < (poola_ptr)->offset; ++idx) \
        if ((poola_ptr)->next[idx] == _ALIVE_VAL)

typedef struct {
    PoolA *pool;
    size_t idx;
    void *data;
} PoolAIt;

static inline PoolAIt poola_iter(PoolA *pool) {
    return (PoolAIt){ pool, (size_t)-1, NULL };
}

static inline bool poola_next(PoolAIt *it) {
    PoolA *pool = it->pool;
    assert(pool && pool->raw);

    while (++it->idx < pool->offset) {
        if (pool->next[it->idx] == (size_t)-1) {
            it->data = (char *)pool->raw + (it->idx * pool->esize);
            return true;
        }
    }

    it->data = NULL;
    return false;
}
