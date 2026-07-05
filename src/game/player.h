#pragma once

#include "common.h"
#include <SDL3/SDL_rect.h>

typedef struct {
    float move_speed;
} PlayerDef;

typedef struct {
    //
    Vec2 pos;

    Vec2 move_dir;
    Vec2 facing_dir;

    // Input
    Vec2 move_input;

    // Components
    size_t drawer;

    // Resources
    size_t image;
} PlayerData;

extern PlayerDef player_def;
extern PlayerData player_data;
