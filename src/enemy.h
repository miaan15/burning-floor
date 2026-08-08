#pragma once

#include "alloc/arena.h"
#include "alloc/pool.h"
#include "math/vec.h"

extern u32 enemy_slime_sprite;
extern const float enemy_slime_move_speed;

typedef struct {
    Vec2 pos;
} EnemySlime;

extern Arena enemy_pools_ar;

extern Pool enemy_slime_pool;

void enemy_init(size_t caps);

void enemy_slime_init(size_t cap);

u32 enemy_slime_new(EnemySlime *data);

void enemy_slime_update();
void enemy_slime_draw();
