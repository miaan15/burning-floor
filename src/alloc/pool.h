#pragma once

#include "arena.h"
#include "macro.h"
#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t esize;

    void *raw;
    size_t cap;

    size_t head;
    size_t len;
    size_t cnt;

    bool *alives;
} pool;

static inline size_t pool_esize(size_t esize) { return MAX(esize, sizeof(size_t)); }
static inline size_t pool_ealign(size_t ealign) { return MAX(ealign, alignof(size_t)); }
size_t pool_scap(size_t esize, size_t cap);

void pool_init(pool *po, size_t esize, size_t cap);
void pool_init_over(pool *po, void *root, size_t esize, size_t cap);
void pool_init_in_arena(pool *po, arena *arena, size_t esize, size_t ealign, size_t cap);

void pool_destroy(pool *po);

size_t pool_new(pool *po, void *data);

bool pool_remv(pool *po, size_t idx);

bool pool_alive(pool *po, size_t idx);

void *pool_ptr(pool *po, size_t idx);

void pool_reset(pool *po);
