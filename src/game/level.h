#pragma once

#include "allocator/allocator.h"
#include "random/random.h"
#include <stdint.h>

typedef int32_t i32;
typedef uint64_t u64;

typedef struct {
    i32 ett;
    size_t drawer;
} EndGate;

typedef struct {
    Random random;
    Arena arena;
    EndGate *endgates;
    size_t endgates_len;
} LevelData;

void level_init(LevelData *data, u64 seed);
void level_destroy(LevelData *data);
