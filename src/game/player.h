#pragma once

#include "alloc/arena.h"
#include "cglm/types.h"
#include "time/time.h"
#include <stddef.h>

typedef struct {
    float move_speed;

    float atk_dur;
    float atk_cd;

    size_t img;
    size_t spr_run_d[4];
    size_t spr_atk_d[4];
    size_t spr_roll_d[4];
    size_t spr_run_u[4];
    size_t spr_atk_u[4];
    size_t spr_roll_u[4];
    size_t spr_run_l[4];
    size_t spr_atk_l[4];
    size_t spr_roll_l[4];
    size_t spr_run_r[4];
    size_t spr_atk_r[4];
    size_t spr_roll_r[4];
} PlayerDef;

typedef struct {
    vec2 pos;

    vec2 run_dir;

    ivec2 face_dir;

    vec2 move_input;
    vec2 atk_dir_inp;
    bool atk_button_inp;

    size_t drwr;

    Arena arena;
    TimelineA ani_run_tl;
    TimelineA ani_atk_tl;
    TimelineA ani_roll_tl;
} Player;

extern PlayerDef player_def;
extern Player player;

void player_init();
void player_destroy();

void player_logic_update();
void player_frame_update();


// typedef struct {
//     float move_speed;
//     float move_ani_delta;
//
//     float attack_duration;
//     float attack_cooldown;
//     float attack_hold_ani_delta;
//     float attack_act_ani_delta;
// } PlayerDef;
//
// typedef struct {
//     //
//     Vec2 pos;
//
//     Vec2 move_dir;
//
//     bool can_attack;
//     bool attack_trigger;
//     bool attacking;
//     float attack_end_time;
//     float attack_off_cd_time;
//     Vec2int attack_dir;
//
//     // Animation
//     Vec2int facing_dir;
//     int cur_move_frame;
//     float last_move_frame_time;
//     int cur_attack_frame;
//     float last_attack_frame_time;
//
//     // Input
//     Vec2 move_input;
//     Vec2int attack_dir_input;
//     bool attack_input;
//
//     // Components
//     size_t drawer;
//     Vec2 drawer_srect_pos;
//
//     // Resources
//     size_t image;
// } PlayerData;
//
// extern PlayerDef player_def;
// extern PlayerData player_data;
//
