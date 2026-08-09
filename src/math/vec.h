#pragma once

#include <math.h>

typedef union {
    struct { float x, y; };
    struct { float w, h; };
    float raw[2];
} vec2;

static inline void vec2_add(vec2 *d, vec2 a, vec2 b) {
    d->x = a.x + b.x;
    d->y = a.y + b.y;
}

static inline void vec2_sub(vec2 *d, vec2 a, vec2 b) {
    d->x = a.x - b.x;
    d->y = a.y - b.y;
}

static inline void vec2_scale(vec2 *d, vec2 a, float s) {
    d->x = a.x * s;
    d->y = a.y * s;
}

static inline float vec2_mag2(vec2 v) {
    return v.x * v.x + v.y * v.y;
}

static inline float vec2_mag(vec2 v) {
    return sqrtf(vec2_mag2(v));
}

static inline void vec2_normalize(vec2 *v) {
    float mag2 = vec2_mag2(*v);
    if (mag2 == 0) return;
    mag2 = sqrtf(mag2);
    v->x /= mag2;
    v->y /= mag2;
}
