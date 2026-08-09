#pragma once

#include "alloc/arena.h"
#include "alloc/pool.h"
#include "macro.h"

typedef struct {
    u32 entity;
    u32 target;
} slime;

extern pool slime_pool;

extern size_t slime_sprite;
extern const float slime_move_speed;

void slime_init(size_t cap);

size_t slime_new(slime *data);

void slime_update();

void slime_draw();

//
extern arena enemy_pools_ar;

void enemy_init(size_t scap);
