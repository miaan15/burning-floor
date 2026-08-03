#include "ring.h"

#include "macro.h"
#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>

#define TAIL_EMPTY ((size_t )-1)

void ring_init(Ring *ri, size_t caps) {
    if (unlikely(ri->raw)) {
        log_err("ring_init(): ring already initialized");
        return;
    }

    ri->raw = malloc(caps);
    ri->caps = caps;
    ri->head = 0;
    ri->tail = TAIL_EMPTY;
    ri->end = ri->caps;
}

void ring_init_over(Ring *ri, void *root, size_t caps) {
    if (unlikely(ri->raw)) {
        log_err("ring_init_over(): ring already initialized");
        return;
    }

    ri->raw = root;
    ri->caps = caps;
    ri->head = 0;
    ri->tail = TAIL_EMPTY;
    ri->end = ri->caps;
}

void ring_destroy(Ring *ri) {
    if (likely(ri->raw)) free(ri->raw);
    memset(ri, 0, sizeof(Ring));
}

void *ring_alloc_raw(Ring *ri, size_t size, size_t align) {
    assert(ri->raw);

    size_t _offs = align_up(ri->head, align);

    if (unlikely(ri->head <= ri->tail && _offs + size > ri->tail)) {
        log_err("ring_alloc(): alloc too much -> return NULL");
        return NULL;
    }

    if (unlikely(ri->tail == TAIL_EMPTY)) ri->tail = ri->head;

    if (unlikely(_offs + size >= ri->caps)) {
        if (unlikely(ri->tail == 0)) {
            log_err("ring_alloc(): alloc too much -> return NULL");
            return NULL;
        }

        ri->end = ri->head;
        _offs = 0;

        if (unlikely(_offs + size > ri->tail)) {
            log_err("ring_alloc(): alloc too much -> return NULL");
            return NULL;
        }
    }

    void *ptr = (char *)ri->raw + _offs;

    ri->head = _offs + size;

    return ptr;
}

void *ring_alloc(Ring *ri, size_t size, size_t align) {
    void *ptr = ring_alloc_raw(ri, size, align);
    if (unlikely(!ptr)) return NULL;
    memset(ptr, 0, size);
    return ptr;
}

bool ring_pop(Ring *ri, size_t size, size_t align) {
    assert(ri->raw);

    if (unlikely(ri->tail == TAIL_EMPTY)) {
        log_err("ring_pop(): ring is empty");
        return false;
    }

    size_t _tail = align_up(ri->tail, align);

    if (unlikely(_tail + size > ri->end)) {
        log_warn("ring_pop(): pop overshot => empty ring");
        ri->tail = TAIL_EMPTY;
        return true;
    }

    if (unlikely(_tail + size == ri->end)) {
        ri->end = ri->caps;
        _tail = 0;
    } else {
        _tail += size;
    }

    if (unlikely(_tail == ri->head)) _tail = TAIL_EMPTY;

    ri->tail = _tail;

    return true;
}

void ring_reset(Ring *ri) {
    assert(ri->raw);
    ri->head = 0;
    ri->tail = TAIL_EMPTY;
    ri->end = ri->caps;
}
