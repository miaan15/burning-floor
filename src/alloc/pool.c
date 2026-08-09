#include "pool.h"

#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdalign.h>
#include <stdlib.h>

size_t pool_scap(size_t esize, size_t cap) {
    return cap * esize + cap;
}

void pool_init(pool *po, size_t esize, size_t cap) {
    assert(!po->raw);

    esize = pool_esize(esize);
    size_t scap = pool_scap(esize, cap);

    po->esize = esize;
    po->raw = malloc(scap);
    po->cap = cap;
    po->head = po->len = po->cnt = 0;
    po->alives = (bool *)((char *)po->raw + cap * esize);
    memset(po->alives, 0, cap);

    log_trace("New Pool from %p to %p: esize = %zu, cap = %zu",
            po->raw, (char *)po->raw + scap, po->esize, po->cap);
}

void pool_init_over(pool *po, void *root, size_t esize, size_t cap) {
    assert(!po->raw);

    esize = pool_esize(esize);
    size_t scap = pool_scap(esize, cap);

    po->esize = esize;
    po->raw = root;
    po->cap = cap;
    po->head = po->len = po->cnt = 0;
    po->alives = (bool *)((char *)po->raw + cap * esize);

    log_trace("New Pool (over) from %p to %p: esize = %zu, cap = %zu",
            po->raw, (char *)po->raw + scap, po->esize, po->cap);
}

void pool_init_in_arena(pool *po, arena *arena, size_t esize, size_t ealign, size_t cap) {
    assert(!po->raw);

    esize = pool_esize(esize);
    ealign = pool_ealign(ealign);
    size_t scap = pool_scap(esize, cap);

    po->esize = esize;
    po->raw = arena_alloc(arena, scap, ealign);
    po->cap = cap;
    po->head = po->len = po->cnt = 0;
    po->alives = (bool *)((char *)po->raw + cap * esize);

    log_trace("New Pool (in arena) from %p to %p: esize = %zu, cap = %zu",
            po->raw, (char *)po->raw + scap, po->esize, po->cap);
}

void pool_destroy(pool *po) {
    if (po->raw) free(po->raw);
    memset(po, 0, sizeof(pool));
}

size_t pool_new(pool *po, void *data) {
    assert(po->raw);

    if (po->len >= po->cap) {
        log_err("pool_new(): too much => 0");
        return 0;
    }

    size_t idx = po->head;

    if (idx == po->len) {
        ++po->len;
        ++po->head;
    } else {
        po->head = *(size_t *)((char *)po->raw + idx * po->esize);
    }

    po->alives[idx] = true;
    ++po->cnt;

    void *ptr = (char *)po->raw + (idx * po->esize);
    if (data) memcpy(ptr, data, po->esize);
    else memset(ptr, 0, po->esize);

    return idx;
}

bool pool_remv(pool *po, size_t idx) {
    assert(po->raw);

    if (!pool_alive(po, idx)) return false;

    *(size_t *)((char *)po->raw + idx * po->esize) = po->head;
    po->alives[idx] = false;

    po->head = idx;

    return true;
}

bool pool_alive(pool *po, size_t idx) {
    assert(po->raw);
    return idx < po->len && po->alives[idx];
}

void *pool_ptr(pool *po, size_t idx) {
    assert(po->raw);
    assert(idx < po->len);
    return (char *)po->raw + (idx * po->esize);
}

void pool_reset(pool *po) {
    assert(po->raw);
    po->head = po->len = po->cnt = 0;
}
