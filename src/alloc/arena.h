#pragma once

#include <stddef.h>

typedef struct {
    void *raw;
    size_t scap;
    size_t offs;
} arena;

void arena_init(arena *ar, size_t caps);
void arena_init_over(arena *ar, void *root, size_t caps);
void arena_init_in_arena(arena *ar, arena *root_ar, size_t caps);

void arena_destroy(arena *ar);

void *arena_alloc_raw(arena *ar, size_t size, size_t align);
void *arena_alloc(arena *ar, size_t size, size_t align);

void arena_reset(arena *ar);
