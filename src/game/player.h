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
    vec2 face_dir;
    bool atk_trggr;
    bool atk_able;
    bool atking;
    vec2 atk_dir;
    float atk_end_time;
    float atk_off_cd_time;

    int cur_ani;
    int last_ani;

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

enum PlayerAni {
    ANI_PLAYER_IDLE = 0,
    ANI_PLAYER_RUN,
    ANI_PLAYER_ATK,
    ANI_PLAYER_ROLL,
};

void player_init();
void player_destroy();

void player_logic_update();
void player_frame_update();
