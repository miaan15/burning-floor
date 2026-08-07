#pragma once

#include <stdio.h>
#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
    #define likely(x)      __builtin_expect(!!(x), 1)
    #define unlikely(x)    __builtin_expect(!!(x), 0)
#else
    #define likely(x)      (x)
    #define unlikely(x)    (x)
#endif

static inline size_t align_up(size_t base, size_t align) {
    return (base + align - 1) & ~(align - 1);
}

typedef uint32_t u32;
typedef uint64_t u64;
