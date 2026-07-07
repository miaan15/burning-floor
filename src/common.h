#pragma once

#include <SDL3/SDL_rect.h>

#define PROJECT_PATH PROJECT_ROOT_DIR
#define SRC_PATH     PROJECT_ROOT_DIR "/src"
#define ASSET_PATH   PROJECT_ROOT_DIR "/asset"

typedef union {
    struct {
        int x, y;
    };
    int data[2];
} Vec2int;

typedef SDL_FPoint Vec2;
typedef SDL_FRect Rect;
