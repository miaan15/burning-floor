#pragma once

#include "alloc/arena.h"
#include "common.h"
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>

#define _POOL_ALIVE_VAL ((uint32_t)-1)

typedef struct {
    uint32_t next;
    uint32_t gen;
} PoolMeta;

static inline size_t pool_msize(size_t esize, size_t cap) {
    size_t size = 0;
    size += cap * esize;
    size = align_up(size, alignof(PoolMeta));
    size += cap * sizeof(PoolMeta);
    return size;
}
static inline size_t pool_malign(size_t ealign) {
    size_t metaalign = alignof(PoolMeta);
    return ealign > metaalign ? ealign : metaalign;
}

typedef struct {
    size_t esize;
    size_t ealign;

    void *raw;
    size_t cap;
    size_t offset;
    size_t cnt;

    size_t head;
    PoolMeta *meta;
} Pool;

void pool_init(Pool *pool, size_t esize, size_t ealign, size_t cap);
static inline Pool pool_make(size_t esize, size_t ealign, size_t cap) {
    Pool p; pool_init(&p, esize, ealign, cap); return p;
}
void pool_destroy(Pool *pool);

Key pool_new(Pool *pool);
bool pool_remv(Pool *pool, Key key);
void pool_reset(Pool *pool);
bool pool_alive(Pool *pool, Key key);
void *pool_get(Pool *pool, Key key);

#define pool_for(pool_ptr, key_var, ptr_var) \
    for (uint32_t _i = 0; _i < (pool_ptr)->offset; ++_i) \
        if ((pool_ptr)->meta[_i].next == _POOL_ALIVE_VAL) \
            for (Key key_var = { _i, (pool_ptr)->meta[_i].gen }, *__k = &key_var; __k; __k = NULL) \
                for (void *ptr_var = (char *)(pool_ptr)->raw + (_i * (pool_ptr)->esize), *__p = ptr_var; __p; __p = NULL)

typedef struct {
    Pool *pool;
    uint32_t _idx;
    Key key;
    void *data;
} PoolIt;

static inline PoolIt pool_iter(Pool *pool) {
    return (PoolIt){ pool, (uint32_t)-1, {0, 0}, NULL };
}

static inline bool pool_next(PoolIt *it) {
    Pool *pool = it->pool;
    assert(pool && pool->raw);

    while (++it->_idx < pool->offset) {
        if (pool->meta[it->_idx].next == _POOL_ALIVE_VAL) {
            it->key = (Key){ it->_idx, pool->meta[it->_idx].gen };
            it->data = (char *)pool->raw + (it->_idx * pool->esize);
            return true;
        }
    }

    it->data = NULL;
    return false;
}

// =============================================================================

typedef struct {
    uint32_t next;
    uint32_t gen;
} PoolAMeta;

typedef struct {
    Arena *arena_ref;

    size_t esize;
    size_t ealign;

    void *raw;
    size_t cap;
    size_t offset;
    size_t cnt;

    size_t head;
    PoolAMeta *meta;
} PoolA;

void poola_init(PoolA *pool, Arena *arena_ref, size_t esize, size_t ealign, size_t cap);
static inline PoolA poola_make(size_t esize, Arena *arena_ref, size_t ealign, size_t cap) {
    PoolA p; poola_init(&p, arena_ref, esize, ealign, cap); return p;
}
void poola_destroy(PoolA *pool);

Key poola_new(PoolA *pool);
bool poola_remv(PoolA *pool, Key key);
void poola_reset(PoolA *pool);
bool poola_alive(PoolA *pool, Key key);
void *poola_get(PoolA *pool, Key key);

#define poola_for(poola_ptr, key_var, ptr_var) \
    for (uint32_t _i = 0; _i < (poola_ptr)->offset; ++_i) \
        if ((poola_ptr)->meta[_i].next == _POOL_ALIVE_VAL) \
            for (Key key_var = { _i, (poola_ptr)->meta[_i].gen }, *__k = &key_var; __k; __k = NULL) \
                for (void *ptr_var = (char *)(poola_ptr)->raw + (_i * (poola_ptr)->esize), *__p = ptr_var; __p; __p = NULL)

typedef struct {
    PoolA *pool;
    uint32_t _idx;
    Key key;
    void *data;
} PoolAIt;

static inline PoolAIt poola_iter(PoolA *pool) {
    return (PoolAIt){ pool, (uint32_t)-1, {0, 0}, NULL };
}

static inline bool poola_next(PoolAIt *it) {
    PoolA *pool = it->pool;
    assert(pool && pool->raw);

    while (++it->_idx < pool->offset) {
        if (pool->meta[it->_idx].next == _POOL_ALIVE_VAL) {
            it->key = (Key){ it->_idx, pool->meta[it->_idx].gen };
            it->data = (char *)pool->raw + (it->_idx * pool->esize);
            return true;
        }
    }

    it->data = NULL;
    return false;
}
