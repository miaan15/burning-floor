#include "arena.h"

#include "macro.h"
#include "log.h"
#include <assert.h>
#include <string.h>
#include <stdalign.h>
#include <stdlib.h>

void arena_init(arena *ar, size_t scap) {
    assert(!ar->raw);

    ar->raw = malloc(scap);
    ar->scap = scap;
    ar->offs = 0;

    log_trace("New Arena from %p to %p: scap = %zu",
            ar->raw, (char *)ar->raw + ar->scap, scap);
}

void arena_init_over(arena *ar, void *root, size_t scap) {
    assert(!ar->raw);

    ar->raw = root;
    ar->scap = scap;
    ar->offs = 0;

    log_trace("New Arena (over) from %p to %p: scap = %zu",
            ar->raw, (char *)ar->raw + ar->scap, scap);
}

void arena_init_in_arena(arena *ar, arena *root_ar, size_t scap) {
    assert(!ar->raw);

    ar->raw = arena_alloc(root_ar, scap, alignof(max_align_t));
    ar->scap = scap;
    ar->offs = 0;

    log_trace("New Arena (in arena) from %p to %p: scap = %zu",
            ar->raw, (char *)ar->raw + ar->scap, scap);
}

void arena_destroy(arena *ar) {
    if (ar->raw) free(ar->raw);
    memset(ar, 0, sizeof(arena));
}

void *arena_alloc(arena *ar, size_t size, size_t align) {
    assert(ar->raw);

    size_t offs = align_up(ar->offs, align);

    if (offs + size > ar->scap) {
        log_err("arena_alloc(): alloc too much -> NULL");
        return NULL;
    }

    ar->offs = offs + size;

    return (char *)ar->raw + offs;
}

void arena_reset(arena *ar) {
    assert(ar->raw);
    ar->offs = 0;
}
