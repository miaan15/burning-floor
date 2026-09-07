#pragma once

#include <stdio.h>
#include <stdint.h>
#include <float.h>

#define MAX(x, y) (x > y ? x : y);
#define MIN(x, y) (x < y ? x : y);

static inline size_t align_up(size_t base, size_t align) {
    return (base + align - 1) & ~(align - 1);
}

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int32_t i32;

typedef _Float32 f32;

typedef struct { u32 raw; } prx32;
typedef struct { u64 raw; } prx64;
