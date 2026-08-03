#include "pool.h"

#include "macro.h"
#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdalign.h>
#include <stdlib.h>

size_t pool_req_size(size_t esize, size_t cap) {
    esize = align_up(esize, sizeof(size_t));
    return cap * esize + cap;
}

void pool_init(Pool *po, size_t esize, size_t cap) {
    if (unlikely(po->raw)) {
        log_err("pool_init(): pool already initialized");
        return;
    }

    esize = align_up(esize, sizeof(size_t));
    size_t caps = pool_req_size(esize, cap);

    po->esize = esize;
    po->raw = malloc(caps);
    po->cap = cap;
    po->head = po->maxi = po->cnt = 0;

    po->meta = (char *)po->raw + (cap * esize);

    memset(po->raw, 0, caps);
}

void pool_init_over(Pool *po, void *root, size_t esize, size_t cap) {
    if (unlikely(po->raw)) {
        log_err("pool_init(): pool already initialized");
        return;
    }

    esize = align_up(esize, sizeof(size_t));
    size_t caps = pool_req_size(esize, cap);

    po->esize = esize;
    po->raw = root;
    po->cap = cap;
    po->head = po->maxi = po->cnt = 0;

    po->meta = (char *)po->raw + (cap * esize);

    memset(po->raw, 0, caps);
}

void pool_destroy(Pool *po) {
    if (po->raw) free(po->raw);
    memset(po, 0, sizeof(Pool));
}

PoolResult pool_new(Pool *po, void *data) {
    assert(po->raw);

    if (unlikely(po->cnt >= po->cap)) {
        log_err("pool_new(): pool full => return NULL");
        return (PoolResult){0};
    }

    size_t i = po->head;
    void *ptr = (char *)po->raw + (i * po->esize);

    if (i == po->maxi) {
        ++po->maxi;
        ++po->head;
    } else {
        po->head = *(size_t *)ptr;
    }

    po->meta[i] |= 1;
    ++po->cnt;

    if (data) {
        memcpy(ptr, data, po->esize);
    } else {
        memset(ptr, 0, po->esize);
    }

    return (PoolResult){ptr, po->meta[i]};
}

bool pool_remv(Pool *po, void *ptr, PoolMeta meta) {
    assert(po->raw);
    assert(((char *)ptr - (char *)po->raw) % po->esize == 0);

    size_t i = ((char *)ptr - (char *)po->raw) / po->esize;

    if (unlikely((i >= po->maxi) || !(po->meta[i] & 1) || (meta != po->meta[i]))) {
        return false;
    }

    *(size_t *)ptr = po->head;

    po->head = i;
    po->meta[i] += (1 << 1);
    po->meta[i] &= ~1;
    --po->cnt;

    return true;
}

bool pool_remv_uc(Pool *po, void *ptr) {
    assert(po->raw);
    assert(((char *)ptr - (char *)po->raw) % po->esize == 0);

    size_t i = ((char *)ptr - (char *)po->raw) / po->esize;

    if (unlikely((i >= po->maxi) || !(po->meta[i] & 1))) {
        return false;
    }

    *(size_t *)ptr = po->head;

    po->head = i;
    po->meta[i] += (1 << 1);
    po->meta[i] &= ~1;
    --po->cnt;

    return true;
}

bool pool_alive(Pool *po, void *ptr, PoolMeta meta) {
    assert(po->raw);
    assert(((char *)ptr - (char *)po->raw) % po->esize == 0);

    size_t i = ((char *)ptr - (char *)po->raw) / po->esize;

    return (i < po->maxi) && (po->meta[i] & 1) && (meta == po->meta[i]);
}

PoolMeta pool_meta(Pool *po, void *ptr) {
    assert(po->raw);
    assert(((char *)ptr - (char *)po->raw) % po->esize == 0);

    size_t i = ((char *)ptr - (char *)po->raw) / po->esize;

    return po->meta[i];
}

void pool_reset(Pool *po) {
    assert(po->raw);
    memset(po->raw, 0, pool_req_size(po->esize, po->cap));
    po->head = po->maxi = po->cnt = 0;
}
