#pragma once

typedef union {
    struct { float x, y, w, h; };
    float raw[4];
} Rect;
