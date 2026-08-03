#pragma once

#include <stddef.h>

typedef struct {
    void *raw;
    size_t caps;
    size_t offs;
} Arena;

void arena_init(Arena *ar, size_t caps);
void arena_init_over(Arena *ar, void *root, size_t caps);

void arena_destroy(Arena *ar);

void *arena_alloc_raw(Arena *ar, size_t size, size_t align);
void *arena_alloc(Arena *ar, size_t size, size_t align);

void arena_reset(Arena *ar);
