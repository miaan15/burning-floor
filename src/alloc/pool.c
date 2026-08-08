#include "pool.h"

#include "macro.h"
#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdalign.h>
#include <stdlib.h>

size_t pool_caps(size_t esize, size_t cap) {
    return align_up(cap * esize, sizeof(size_t)) + (cap * sizeof(size_t));
}

void pool_init(Pool *po, size_t esize, size_t cap) {
    if (unlikely(po->raw)) {
        log_err("pool_init(): pool already initialized");
        return;
    }

    size_t caps = pool_caps(esize, cap);

    po->esize = esize;
    po->raw = malloc(caps);
    po->cap = cap;
    po->head = po->maxi = po->cnt = 0;

    po->meta = (size_t *)((char *)po->raw + align_up(cap * esize, sizeof(size_t)));
    memset(po->meta, 0, cap * sizeof(size_t));

    log_trace("New Pool from %p to %p: esize = %zu, cap = %zu",
            po->raw, (char *)po->raw + caps, po->esize, po->cap);
}

void pool_init_over(Pool *po, void *root, size_t esize, size_t cap) {
    if (unlikely(po->raw)) {
        log_err("pool_init(): pool already initialized");
        return;
    }

    size_t caps = pool_caps(esize, cap);

    po->esize = esize;
    po->raw = root;
    po->cap = cap;
    po->head = po->maxi = po->cnt = 0;

    po->meta = (size_t *)((char *)po->raw + align_up(cap * esize, sizeof(size_t)));
    memset(po->meta, 0, cap * sizeof(size_t));

    log_trace("New Pool (over) from %p to %p: esize = %zu, cap = %zu",
            po->raw, (char *)po->raw + caps, po->esize, po->cap);
}

void pool_destroy(Pool *po) {
    if (po->raw) free(po->raw);
    memset(po, 0, sizeof(Pool));
}

size_t pool_new(Pool *po, void *data) {
    assert(po->raw);

    if (unlikely(po->cnt >= po->cap)) {
        log_err("pool_new(): pool full => return -1");
        return (u32)-1;
    }

    size_t i = po->head;

    if (i == po->maxi) {
        ++po->maxi;
        ++po->head;
    } else {
        po->head = po->meta[i];
    }

    po->meta[i] = (u32)-1;
    ++po->cnt;

    void *ptr = (char *)po->raw + (i * po->esize);
    if (data) {
        memcpy(ptr, data, po->esize);
    } else {
        memset(ptr, 0, po->esize);
    }

    return i;
}

bool pool_remv(Pool *po, size_t idx) {
    assert(po->raw);

    if (unlikely(idx >= po->maxi || po->meta[idx] != (u32)-1)) {
        return false;
    }

    pool_remv_uc(po, idx);

    return true;
}

void pool_remv_uc(Pool *po, size_t idx) {
    assert(po->raw);
    assert(idx < po->maxi);

    po->meta[idx] = po->head;
    po->head = idx;

    void *ptr = (char *)po->raw + (idx * po->esize);
    memset(ptr, 0, po->esize);

    --po->cnt;
}

bool pool_alive(Pool *po, size_t idx) {
    assert(po->raw);
    return idx < po->maxi && po->meta[idx] == (u32)-1;
}

void *pool_ptr(Pool *po, size_t idx) {
    assert(po->raw);
    assert(idx < po->maxi);
    return (char *)po->raw + (idx * po->esize);
}

size_t pool_index(Pool *po, void *data) {
    assert(po->raw);
    assert((char *)data >= (char *)po->raw);
    assert(po->maxi > 0 && (char *)data <= (char *)po->raw + ((po->maxi - 1) * po->esize));
    assert(((char *)data - (char *)po->raw) % po->esize == 0);
    return ((char *)data - (char *)po->raw) / po->esize;
}

void pool_reset(Pool *po) {
    assert(po->raw);
    memset(po->meta, 0, po->cap * sizeof(size_t));
    po->head = po->maxi = po->cnt = 0;
}
