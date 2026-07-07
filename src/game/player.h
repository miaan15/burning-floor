#pragma once

#include "common.h"

typedef struct {
    float move_speed;
    float move_ani_delta;
} PlayerDef;

typedef struct {
    //
    Vec2 pos;

    Vec2 move_dir;
    Vec2int facing_dir;

    // Input
    Vec2 move_input;

    // Components
    size_t drawer;
    Vec2 drawer_srect_pos;
    float last_frame_update;
    int cur_frame;

    // Resources
    size_t image;
} PlayerData;

extern PlayerDef player_def;
extern PlayerData player_data;
