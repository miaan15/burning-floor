#include "pool.h"
#include <assert.h>
#include "log/log.h"

void _pool_set_alive(Pool *pool, size_t idx, bool v) {
    char *bitset = (char *)(pool->alive);
    size_t byte_idx = idx / 8;
    size_t bit_idx = idx % 8;
    bitset[byte_idx] = (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
}
bool _pool_is_alive(Pool *pool, size_t idx) {
    char *bitset = (char *)(pool->alive);
    return (bitset[idx / 8] >> (idx % 8)) & 1;
}

void pool_init(Pool *pool, size_t esize, size_t cap) {
    if (esize < sizeof(i32)) {
        log_warn("pool_init(): esize (which is %zu) must be > 4 => change it to 4", esize);
        esize = 4;
    }
    pool->esize = esize;
    pool->cap = cap;
    pool->data = malloc((cap * esize) + ((cap + 7) / 8));
    pool->alive = (char *)pool->data + (cap * esize);
    pool->offset = pool->cnt = pool->next = 0;
}

Pool pool_make(size_t esize, size_t cap) {
    Pool p;
    pool_init(&p, esize, cap);
    return p;
}

void pool_destroy(Pool *pool) {
    free(pool->data);
    pool->esize = pool->cap = pool->offset = pool->cnt = pool->next = 0;
}

i32 pool_new(Pool *pool) {
    assert(pool->data);
    if (pool->cnt >= pool->cap) {
        log_err("pool_add(): full capacity (which is %zu) => return -1", pool->cap);
        return -1;
    }

i32 obj = (size_t)pool->next;
    if ((size_t)obj == pool->offset) {
        ++pool->offset;
        ++pool->next;
    } else {
        memcpy(&pool->next, (char *)pool->data + (obj * pool->esize), sizeof(i32));
    }

    _pool_set_alive(pool, obj, true);
    ++pool->cnt;

    memset((char *)pool->data + (obj * pool->esize), 0, pool->esize);

    return obj;
}

bool pool_remv(Pool *pool, i32 obj) {
    assert(pool->data);
    if (obj < 0 || obj >= pool->offset) {
        log_err("pool_remv(): obj is out of bounds (requested %d)", obj);
        return false;
    }
    if (!_pool_is_alive(pool, obj)) {
        log_warn("pool_remv(): obj is removed already (requested %d)", obj);
        return false;
    }

    memcpy((char *)pool->data + (obj * pool->esize), &pool->next, sizeof(i32));
    pool->next = obj;

    _pool_set_alive(pool, obj, false);
    --pool->cnt;

    return true;
}

void pool_reset(Pool *pool) {
    assert(pool->data);
    memset(pool->data, 0, pool->cap * pool->esize);
    memset(pool->alive, 0, (pool->cap + 7) / 8);
    pool->offset = pool->cnt = pool->next = 0;
}

bool pool_alive(Pool *pool, i32 obj) {
    if (obj < 0 || obj >= pool->offset) return false;
    return _pool_is_alive(pool, obj);
}

void *pool_get(Pool *pool, i32 obj) {
    assert(pool->data);
    if (obj < 0 || obj >= pool->offset) {
        log_err("pool_get(): obj is out of bounds (requested %d) => return stub", obj);
        return pool->data;
    }
    if (!_pool_is_alive(pool, obj)) {
        log_err("pool_get(): obj is removed (requested %d) => return stub", obj);
        return pool->data;
    }

    return (char *)pool->data + (obj * pool->esize);
}

// =============================================================================
// Pool Arena

void _poola_set_alive(PoolA *pool, size_t idx, bool v) {
    char *bitset = (char *)(pool->alive);
    size_t byte_idx = idx / 8;
    size_t bit_idx = idx % 8;
    bitset[byte_idx] = (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
}
bool _poola_is_alive(PoolA *pool, size_t idx) {
    char *bitset = (char *)(pool->alive);
    return (bitset[idx / 8] >> (idx % 8)) & 1;
}

void poola_init(PoolA *pool, Arena *arena, size_t esize, size_t cap) {
    if (esize < sizeof(i32)) {
        log_warn("pool_init(): esize (which is %zu) must be > 4 => change it to 4", esize);
        esize = 4;
    }
    pool->arena = pool->arena;
    pool->esize = esize;
    pool->cap = cap;
    pool->data = arena_alloc(pool->arena, cap * esize, 1);
    pool->alive = arena_alloc(pool->arena, (cap + 7) / 8, 1);
    pool->offset = pool->cnt = pool->next = 0;
}

PoolA poola_make(Arena *arena, size_t esize, size_t cap) {
    PoolA p;
    poola_init(&p, arena, esize, cap);
    return p;
}

i32 poola_new(PoolA *pool) {
    assert(pool->arena->buffer);
    if (pool->cnt >= pool->cap) {
        log_err("pool_add(): full capacity (which is %zu) => return -1", pool->cap);
        return -1;
    }

    i32 obj = (size_t)pool->next;
    if ((size_t)obj == pool->offset) {
        ++pool->offset;
        ++pool->next;
    } else {
        memcpy(&pool->next, (char *)pool->data + (obj * pool->esize), sizeof(i32));
    }

    _poola_set_alive(pool, obj, true);
    ++pool->cnt;

    memset((char *)pool->data + (obj * pool->esize), 0, pool->esize);

    return obj;
}

bool poola_remv(PoolA *pool, i32 obj) {
    assert(pool->arena->buffer);
    if (obj < 0 || obj >= pool->offset) {
        log_err("pool_remv(): obj is out of bounds (requested %d)", obj);
        return false;
    }
    if (!_poola_is_alive(pool, obj)) {
        log_warn("pool_remv(): obj is removed already (requested %d)", obj);
        return false;
    }

    memcpy((char *)pool->data + (obj * pool->esize), &pool->next, sizeof(i32));
    pool->next = obj;

    _poola_set_alive(pool, obj, false);
    --pool->cnt;

    return true;
}

void poola_reset(PoolA *pool) {
    assert(pool->arena->buffer);
    memset(pool->data, 0, pool->cap * pool->esize);
    memset(pool->alive, 0, (pool->cap + 7) / 8);
    pool->offset = pool->cnt = pool->next = 0;
}

bool poola_alive(PoolA *pool, i32 obj) {
    if (obj < 0 || obj >= pool->offset) return false;
    return _poola_is_alive(pool, obj);
}

void *poola_get(PoolA *pool, i32 obj) {
    assert(pool->arena->buffer);
    if (obj < 0 || obj >= pool->offset) {
        log_err("pool_get(): obj is out of bounds (requested %d) => return stub", obj);
        return pool->data;
    }
    if (!_poola_is_alive(pool, obj)) {
        log_err("pool_get(): obj is removed (requested %d) => return stub", obj);
        return pool->data;
    }

    return (char *)pool->data + (obj * pool->esize);
}
