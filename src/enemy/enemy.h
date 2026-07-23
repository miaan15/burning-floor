#pragma once

#include "alloc/arena.h"
#include "cglm/types.h"
#include "common.h"
#include "pool/pool.h"

typedef struct {
    float jump_vel;
    float jump_delay;
    float jump_dur;

    float cd;
    float cd_vari;
} EnemySlimeDef;

typedef struct {

} EnemySlime;

enum EnemyType {
    ENEMY_NONE = 0,
    ENEMY_SLIME
};

typedef struct {
    int type;
    union {
        EnemySlime slime;
    };

    Key target;

    Key ett;
    Key drwr;
} EnemyIns;

typedef struct {
    size_t img;
    size_t spr_slime[5];
    size_t spr_rock[3];
    size_t spr_fly[4];

    Arena arena;

    PoolA enemy_pool;
} EnemyMng;

extern EnemyMng enemy_mng;

void enemy_mng_init(EnemyMng *mng, size_t cap);
void enemy_mng_destroy(EnemyMng *mng);
void enemy_mng_update(EnemyMng *mng);

Key enemy_new(EnemyMng *mng);
void enemy_remv(EnemyMng *mng, Key enemy);

EnemyIns *enemy_get(EnemyMng *mng, Key enemy);

void enemy_slime_init(EnemyMng *mng, Key enemy, vec2 pos);
void enemy_slime_update(EnemyMng *mng, Key enemy);
