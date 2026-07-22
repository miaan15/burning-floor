#pragma once

#include <SDL3/SDL_rect.h>

#define PROJECT_PATH PROJECT_ROOT_DIR
#define SRC_PATH     PROJECT_ROOT_DIR "/src"
#define ASSET_PATH   PROJECT_ROOT_DIR "/asset"

typedef struct { uint32_t idx, gen; } Key;

static inline uint64_t key2u64(Key k) {
    return ((uint64_t)k.gen << 32) | (uint64_t)k.idx;
}
