#include "pool.h"
#include <assert.h>
#include "log/log.h"

#define _ALIVE_VAL ((size_t)-1)

void pool_init(Pool *pool, size_t esize, size_t ealign, size_t cap) {
    pool->esize = esize;
    pool->ealign = ealign;
    pool->cap = cap;
    size_t size = pool_msize(esize, cap);
    pool->raw = malloc(size);
    pool->next = (size_t *)((char *)pool->raw + align_up(cap * esize, alignof(size_t)));
    memset(pool->raw, 0, size);
    pool->offset = pool->cnt = pool->head = 0;
    memset(pool->raw, 0, size);
}

void pool_destroy(Pool *pool) {
    free(pool->raw);
    pool->raw = NULL;
    pool->esize = pool->cap = pool->offset = pool->cnt = pool->head = 0;
}

size_t pool_new(Pool *pool) {
    assert(pool->raw);
    if (pool->cnt >= pool->cap) {
        log_err("pool_new(): full capacity (which is %zu) => return 0", pool->cap);
        return 0;
    }

    size_t obj = pool->head;

    if (obj == pool->offset) {
        ++pool->offset;
        ++pool->head;
    } else {
        pool->head = pool->next[obj];
    }
    pool->next[obj] = _ALIVE_VAL;
    ++pool->cnt;

    return obj;
}

bool pool_remv(Pool *pool, size_t obj) {
    assert(pool->raw);
    if (obj >= pool->offset || pool->next[obj] != _ALIVE_VAL) {
        log_err("pool_remv(): obj %zu is not existed (removed or out of bounds)", obj);
        return false;
    }

    pool->next[obj] = pool->head;
    pool->head = obj;
    --pool->cnt;

    memset((char *)pool->raw + (obj * pool->esize), 0, pool->esize);

    return true;
}

void pool_reset(Pool *pool) {
    assert(pool->raw);
    memset(pool->raw, 0, pool_msize(pool->esize, pool->cap));
    pool->offset = pool->cnt = pool->head = 0;
}

bool pool_alive(Pool *pool, size_t obj) {
    return obj < pool->offset && pool->next[obj] == _ALIVE_VAL;
}

void *pool_get(Pool *pool, size_t obj) {
    assert(pool->raw);
    if (obj >= pool->offset || pool->next[obj] != _ALIVE_VAL) {
        log_err("pool_get(): obj %zu is not existed (removed or out of bounds) => return stub", obj);
        return pool->raw;
    }
    return (char *)pool->raw + (obj * pool->esize);
}

// =============================================================================
// Pool Arena
void poola_init(PoolA *pool, Arena *arena, size_t esize, size_t ealign, size_t cap) {
    pool->arena = arena;
    pool->esize = esize;
    pool->ealign = ealign;
    pool->cap = cap;
    size_t size = pool_msize(esize, cap);
    size_t align = pool_malign(ealign);
    pool->raw = arena_alloc(arena, size, align);
    pool->next = (size_t *)((char *)pool->raw + align_up(cap * esize, alignof(size_t)));
    memset(pool->raw, 0, size);
    pool->offset = pool->cnt = pool->head = 0;
}

void poola_destroy(PoolA *pool) {
    free(pool->raw);
    pool->raw = NULL;
    pool->esize = pool->cap = pool->offset = pool->cnt = pool->head = 0;
}

size_t poola_new(PoolA *pool) {
    assert(pool->raw);
    if (pool->cnt >= pool->cap) {
        log_err("poola_new(): full capacity (which is %zu) => return 0", pool->cap);
        return 0;
    }

    size_t obj = pool->head;

    if (obj == pool->offset) {
        ++pool->offset;
        ++pool->head;
    } else {
        pool->head = pool->next[obj];
    }
    pool->next[obj] = _ALIVE_VAL;
    ++pool->cnt;

    return obj;
}

bool poola_remv(PoolA *pool, size_t obj) {
    assert(pool->raw);
    if (obj >= pool->offset || pool->next[obj] != _ALIVE_VAL) {
        log_err("poola_remv(): obj %zu is not existed (removed or out of bounds)", obj);
        return false;
    }

    pool->next[obj] = pool->head;
    pool->head = obj;
    --pool->cnt;

    memset((char *)pool->raw + (obj * pool->esize), 0, pool->esize);

    return true;
}

void poola_reset(PoolA *pool) {
    assert(pool->raw);
    memset(pool->raw, 0, poola_msize(pool->esize, pool->cap));
    pool->offset = pool->cnt = pool->head = 0;
}

bool poola_alive(PoolA *pool, size_t obj) {
    return obj < pool->offset && pool->next[obj] == _ALIVE_VAL;
}

void *poola_get(PoolA *pool, size_t obj) {
    assert(pool->raw);
    if (obj >= pool->offset || pool->next[obj] != _ALIVE_VAL) {
        log_err("poola_get(): obj %zu is not existed (removed or out of bounds) => return stub", obj);
        return pool->raw;
    }
    return (char *)pool->raw + (obj * pool->esize);
}
