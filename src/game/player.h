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
    size_t sprite_drawer;
} PlayerData;

void player_init() {
    
}
