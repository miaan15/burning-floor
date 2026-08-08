#include "arena.h"

#include "macro.h"
#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdalign.h>
#include <stdlib.h>

void arena_init(Arena *ar, size_t caps) {
    if (unlikely(ar->raw)) {
        log_err("arena_init(): arena already initialized");
        return;
    }
    ar->raw = malloc(caps);
    ar->caps = caps;
    ar->offs = 0;

    log_trace("New Arena from %p to %p: caps = %zu",
            ar->raw, (char *)ar->raw + ar->caps, caps);
}

void arena_init_over(Arena *ar, void *root, size_t caps) {
    if (unlikely(ar->raw)) {
        log_err("arena_init_over(): arena already initialized");
        return;
    }
    ar->raw = root;
    ar->caps = caps;
    ar->offs = 0;

    log_trace("New Arena (over) from %p to %p: caps = %zu",
            ar->raw, (char *)ar->raw + ar->caps, caps);
}

void arena_destroy(Arena *ar) {
    if (likely(ar->raw)) free(ar->raw);
    memset(ar, 0, sizeof(Arena));
}

void *arena_alloc_raw(Arena *ar, size_t size, size_t align) {
    assert(ar->raw);

    size_t _offs = align_up(ar->offs, align);

    if (unlikely(_offs + size > ar->caps)) {
        log_err("arena_alloc(): alloc too much -> return NULL");
        return NULL;
    }

    ar->offs = _offs + size;

    return (char *)ar->raw + _offs;
}

void *arena_alloc(Arena *ar, size_t size, size_t align) {
    void *ptr = arena_alloc_raw(ar, size, align);
    if (unlikely(!ptr)) return NULL;
    memset(ptr, 0, size);
    return ptr;
}

void arena_reset(Arena *ar) {
    assert(ar->raw);
    ar->offs = 0;
}
