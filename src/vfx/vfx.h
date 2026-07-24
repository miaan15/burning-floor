#pragma once

#include "alloc/arena.h"
#include "pool/pool.h"

typedef struct {

} VfxIns;

typedef struct {
    Arena arena;

    PoolA vfx_pool;

    Arena data_arena;
} VfxMng;
