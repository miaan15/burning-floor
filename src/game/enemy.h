#pragma once

#include "allocator/allocator.h"
#include "common.h"

typedef struct {
    float move_speed;
    size_t image;
} EnemyMeleeDef;

typedef struct {
    //
    Vec2 pos;

    bool has_target;
    Vec2 target;

    Vec2 move_dir;
    Vec2 facing_dir;

    // Components
    size_t drawer;
} EnemyMeleeData;

typedef enum {
    ENEMY_NONE,
    ENEMY_MELEE
} EnemyType;

typedef struct {
    uint8_t type;
    union {
        EnemyMeleeData melee_data;
    };
} EnemyData;

typedef struct {
    Arena arena;

    EnemyData *enemies;
    size_t enemies_offset;
    size_t enemies_count;
    size_t enemies_cap;
    size_t enemies_next_id;
    void *enemies_deleted;
} EnemyMng;

extern EnemyMeleeDef enemy_melee_def;

void enemy_defs_init();

extern EnemyMng enemy_mng;

void enemy_mng_init(EnemyMng *mng, size_t enemies_cap);
void enemy_mng_destroy(EnemyMng *mng);

size_t enemy_make(EnemyMng *mng, uint8_t type);
void enemy_remove(EnemyMng *mng, size_t enemy);

void enemy_melee_init(EnemyMng *mng, size_t enemy);
void enemy_melee_update_behavior(EnemyMng *mng, size_t enemy);
void enemy_melee_update_render(EnemyMng *mng, size_t enemy);
void enemy_melee_draw(EnemyMng *mng, size_t enemy);
