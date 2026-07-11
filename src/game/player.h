#pragma once

#include "common.h"

typedef struct {
    float move_speed;
    float move_ani_delta;

    float attack_duration;
    float attack_cooldown;
    float attack_hold_ani_delta;
    float attack_act_ani_delta;
} PlayerDef;

typedef struct {
    //
    Vec2 pos;

    Vec2 move_dir;

    bool can_attack;
    bool attack_trigger;
    bool attacking;
    float attack_end_time;
    float attack_off_cd_time;
    Vec2int attack_dir;

    // Animation
    Vec2int facing_dir;
    int cur_move_frame;
    float last_move_frame_time;
    int cur_attack_frame;
    float last_attack_frame_time;

    // Input
    Vec2 move_input;
    bool attack_input;

    // Components
    size_t drawer;
    Vec2 drawer_srect_pos;

    // Resources
    size_t image;
} PlayerData;

extern PlayerDef player_def;
extern PlayerData player_data;
