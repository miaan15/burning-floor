#pragma once

#include <stdalign.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static inline size_t align_up(size_t base, size_t align) {
    return (base + align - 1) & ~(align - 1);
}

typedef struct {
    void *buffer;
    size_t offset;
    size_t cap;
} Arena;

static inline void arena_init(Arena *ar, size_t size) {
    ar->buffer = malloc(size);
    ar->offset = 0;
    ar->cap = size;
    memset(ar->buffer, 0, size);
}

static inline Arena arena_make(size_t size) {
    return (Arena){ .buffer = malloc(size), .offset = 0, .cap = size };
}

static inline void arena_destroy(Arena *ar) {
    if (ar->buffer) free(ar->buffer);
    ar->buffer = NULL;
    ar->offset = ar->cap = 0;
}

static inline void arena_reset(Arena *ar) {
    memset(ar->buffer, 0, ar->offset);
    ar->offset = 0;
}

static inline void *arena_alloc(Arena *ar, size_t size, size_t align) {
    size_t aligned = align_up(ar->offset, align);
    if (aligned + size > ar->cap) return NULL;

    ar->offset = aligned + size;

    char *ptr = (char *)ar->buffer + aligned;
    return ptr;
}

static inline void arena_child_init(Arena *ar, size_t size, Arena *par) {
    ar->buffer = arena_alloc(par, size, alignof(max_align_t));
    ar->offset = 0;
    ar->cap = size;
    memset(ar->buffer, 0, size);
}
