#pragma once

#include "macro.h"
#include <math.h>

typedef union {
    struct { float x, y; };
    struct { float w, h; };
    float raw[2];
} Vec2;

static inline void vec2_add(Vec2 *d, Vec2 a, Vec2 b) {
    d->x = a.x + b.x;
    d->y = a.y + b.y;
}

static inline void vec2_sub(Vec2 *d, Vec2 a, Vec2 b) {
    d->x = a.x - b.x;
    d->y = a.y - b.y;
}

static inline void vec2_scale(Vec2 *d, Vec2 a, float s) {
    d->x = a.x * s;
    d->y = a.y * s;
}

static inline float vec2_mag2(Vec2 v) {
    return v.x * v.x + v.y * v.y;
}

static inline float vec2_mag(Vec2 v) {
    return sqrtf(vec2_mag2(v));
}

static inline void vec2_normalize(Vec2 *v) {
    float mag = vec2_mag2(*v);
    if (unlikely(mag == 0)) return;
    mag = sqrtf(mag);
    v->x /= mag;
    v->y /= mag;
}
