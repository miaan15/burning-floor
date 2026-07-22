#include "pool.h"
#include <assert.h>
#include "log/log.h"

void pool_init(Pool *pool, size_t esize, size_t ealign, size_t cap) {
    pool->esize = esize;
    pool->ealign = ealign;
    pool->cap = cap;

    size_t size = pool_msize(esize, cap);
    pool->raw = malloc(size);
    pool->meta = (PoolMeta *)((char *)pool->raw + align_up(cap * esize, alignof(PoolMeta)));

    pool->offset = pool->cnt = pool->head = 0;
}

void pool_destroy(Pool *pool) {
    free(pool->raw);
    pool->raw = NULL;
    pool->meta = NULL;
    pool->esize = pool->cap = pool->offset = pool->cnt = pool->head = 0;
}

Key pool_new(Pool *pool) {
    assert(pool->raw);
    if (pool->cnt >= pool->cap) {
        log_err("pool_new(): full capacity (which is %u) => return stub", pool->cap);
        return (Key){0, 0};
    }

    size_t idx = pool->head;

    if (idx == pool->offset) {
        ++pool->offset;
        ++pool->head;
    } else {
        pool->head = pool->meta[idx].next;
    }

    pool->meta[idx].next = _POOL_ALIVE_VAL;

    ++pool->cnt;

    return (Key){ idx, pool->meta[idx].gen };
}

bool pool_remv(Pool *pool, Key key) {
    assert(pool->raw);
    if (!pool_alive(pool, key)) {
        log_err("pool_remv(): key %u:%u does not exist (removed, out of bounds, or stale)", key.idx, key.gen);
        return false;
    }

    pool->meta[key.idx].next = pool->head;
    pool->head = key.idx;

    pool->meta[key.idx].gen++;

    --pool->cnt;

    memset((char *)pool->raw + (key.idx * pool->esize), 0, pool->esize);

    return true;
}

void pool_reset(Pool *pool) {
    assert(pool->raw);
    memset(pool->raw, 0, pool_msize(pool->esize, pool->cap));
    pool->offset = pool->cnt = pool->head = 0;
}

bool pool_alive(Pool *pool, Key key) {
    return key.idx < pool->offset &&
           pool->meta[key.idx].next == _POOL_ALIVE_VAL &&
           pool->meta[key.idx].gen == key.gen;
}

void *pool_get(Pool *pool, Key key) {
    assert(pool->raw);
    if (!pool_alive(pool, key)) {
        log_err("pool_get(): key %u:%u does not exist => return stub", key.idx, key.gen);
        return pool->raw;
    }
    return (char *)pool->raw + (key.idx * pool->esize);
}

// =============================================================================
// Pool Arena

void poola_init(PoolA *pool, Arena *arena_ref, size_t esize, size_t ealign, size_t cap) {
    pool->arena_ref = arena_ref;
    pool->esize = esize;
    pool->ealign = ealign;
    pool->cap = cap;

    size_t size = pool_msize(esize, cap);
    pool->raw = arena_alloc(arena_ref, size, pool_malign(ealign));
    pool->meta = (PoolAMeta *)((char *)pool->raw + align_up(cap * esize, alignof(PoolAMeta)));

    pool->offset = pool->cnt = pool->head = 0;
}

Key poola_new(PoolA *pool) {
    assert(pool->raw);
    if (pool->cnt >= pool->cap) {
        log_err("poola_new(): full capacity (which is %u) => return stub", pool->cap);
        return (Key){0, 0};
    }

    size_t idx = pool->head;

    if (idx == pool->offset) {
        ++pool->offset;
        ++pool->head;
    } else {
        pool->head = pool->meta[idx].next;
    }

    pool->meta[idx].next = _POOL_ALIVE_VAL;

    ++pool->cnt;

    return (Key){ idx, pool->meta[idx].gen };
}

bool poola_remv(PoolA *pool, Key key) {
    assert(pool->raw);
    if (!poola_alive(pool, key)) {
        log_err("poola_remv(): key %u:%u does not exist (removed, out of bounds, or stale)", key.idx, key.gen);
        return false;
    }

    pool->meta[key.idx].next = pool->head;
    pool->head = key.idx;

    pool->meta[key.idx].gen++;

    --pool->cnt;

    memset((char *)pool->raw + (key.idx * pool->esize), 0, pool->esize);

    return true;
}

void poola_reset(PoolA *pool) {
    assert(pool->raw);
    memset(pool->raw, 0, pool_msize(pool->esize, pool->cap));
    pool->offset = pool->cnt = pool->head = 0;
}

bool poola_alive(PoolA *pool, Key key) {
    return key.idx < pool->offset &&
           pool->meta[key.idx].next == _POOL_ALIVE_VAL &&
           pool->meta[key.idx].gen == key.gen;
}

void *poola_get(PoolA *pool, Key key) {
    assert(pool->raw);
    if (!poola_alive(pool, key)) {
        log_err("poola_get(): key %u:%u does not exist => return stub", key.idx, key.gen);
        return pool->raw;
    }
    return (char *)pool->raw + (key.idx * pool->esize);
}
