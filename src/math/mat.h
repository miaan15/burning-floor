#pragma once

#include "macro.h"
#include <stdbool.h>

typedef union {
    struct { f32 x, y, w, h; };
    f32 raw[4];
    f32 table[2][2];
} mat2;

typedef mat2 rect;

static inline bool mat2_intersect(mat2 a, mat2 b) {
    return (a.x < b.x + b.w) &&
           (a.x + a.w > b.x) &&
           (a.y < b.y + b.h) &&
           (a.y + a.h > b.y);
}
