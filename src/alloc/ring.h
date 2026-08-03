#pragma once

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    void *raw;
    size_t caps;
    size_t head, tail;
    size_t end;
} Ring;
// tail == -1 => ring is empty

void ring_init(Ring *ri, size_t caps);
void ring_init_over(Ring *ri, void *root, size_t caps);

void ring_destroy(Ring *ri);

void *ring_alloc_raw(Ring *ri, size_t size, size_t align);
void *ring_alloc(Ring *ri, size_t size, size_t align);

bool ring_pop(Ring *ri, size_t size, size_t align);

void ring_reset(Ring *ri);
