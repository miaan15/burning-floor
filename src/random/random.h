#pragma once

#include <stdint.h>

typedef uint64_t u64;
typedef u64 Random;

static inline void _random_next(Random *rand) {
    *rand += 0x9e3779b97f4a7c15ULL;
}

static inline u64 random_u(Random *rand) {
    _random_next(rand);
    u64 z = *rand;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static inline double random_f(Random *rand) {
    u64 z = random_u(rand);
    return (double)z / (double)UINT64_MAX;
}

static inline void random_reset(Random *rand, u64 seed) { *rand = seed; }
