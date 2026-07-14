#include "pool.h"
#include <assert.h>
#include <stdbool.h>
#include "log/log.h"

void _set_alive(Pool *pool, size_t idx, bool v) {
    char *bitset = (char *)(pool->alive);
    size_t byte_idx = idx / 8;
    size_t bit_idx = idx % 8;
    bitset[byte_idx] = (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
}
bool _is_alive(Pool *pool, size_t idx) {
    char *bitset = (char *)(pool->alive);
    return (bitset[idx / 8] >> (idx % 8)) & 1;
}

void pool_init(Pool *pool, Arena *arena, size_t esize, size_t cap) {
    if (esize < sizeof(i32)) {
        log_err("pool_init(): esize (which is %zu) must be > 4 => change it to 4", esize);
        esize = 4;
    }
    pool->arena = pool->arena;
    pool->esize = esize;
    pool->cap = cap;
    pool->data = arena_alloc(pool->arena, cap * esize, 1);
    pool->alive = arena_alloc(pool->arena, (cap + 7) / 8, 1);
    pool->offset = pool->cnt = pool->next = 0;
}

i32 pool_add(Pool *pool) {
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

    _set_alive(pool, obj, true);
    ++pool->cnt;

    memset((char *)pool->data + (obj * pool->esize), 0, pool->esize);

    return obj;
}

void pool_remv(Pool *pool, i32 obj) {
    assert(pool->arena->buffer);
    if (obj < 0 || obj >= pool->offset) {
        log_err("pool_remv(): obj is out of bounds (requested %d)", obj);
        return;
    }
    if (!_is_alive(pool, obj)) {
        log_err("pool_remv(): obj is removed already (requested %d)", obj);
        return;
    }

    memcpy((char *)pool->data + (obj * pool->esize), &pool->next, sizeof(i32));
    pool->next = obj;

    _set_alive(pool, obj, false);
    --pool->cnt;
}

void pool_reset(Pool *pool) {
    assert(pool->arena->buffer);
    memset(pool->data, 0, pool->cap * pool->esize);
    memset(pool->alive, 0, (pool->cap + 7) / 8);
    pool->offset = pool->cnt = pool->next = 0;
}

void *pool_get(Pool *pool, i32 obj) {
    assert(pool->arena->buffer);
    if (obj < 0 || obj >= pool->offset) {
        log_err("pool_get(): obj is out of bounds (requested %d) => return stub", obj);
        return pool->data;
    }
    if (!_is_alive(pool, obj)) {
        log_err("pool_get(): obj is removed (requested %d) => return stub", obj);
        return pool->data;
    }

    return (char *)pool->data + (obj * pool->esize);
}
