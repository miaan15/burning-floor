#include "enemy.h"

#include "draw.h"
#include "entity.h"
#include "log.h"
#include <stdalign.h>
#include <context.h>

u32 enemy_slime_sprite = 0;
const float enemy_slime_move_speed = .2;

Arena enemy_pools_ar = {0};

Pool enemy_slime_pool = {0};

void enemy_init(size_t caps) {
    arena_init_over(&enemy_pools_ar, arena_alloc(&global_ar, caps, alignof(u64)), caps);
    log_debug("New Enemy Pools from %p to %p: caps = %zu",
            enemy_pools_ar.raw, (char *)enemy_pools_ar.raw + enemy_pools_ar.caps, enemy_pools_ar.caps);
}

void enemy_slime_init(size_t cap) {
    size_t caps = pool_caps(cap, sizeof(EnemySlime));
    pool_init_over(&enemy_slime_pool, arena_alloc(&enemy_pools_ar, caps, alignof(u64)), sizeof(EnemySlime), caps);
    log_debug("New Enemy Slime Pool from %p to %p: cap = %zu",
            enemy_slime_pool.raw, (char *)enemy_slime_pool.raw + caps, cap);
}

u32 enemy_slime_new(EnemySlime *data) {
    u32 slime = pool_new(&enemy_slime_pool, data);
    if (slime == (u32)-1) log_err("enemy_slime_new(): ");

    log_debug("Create EnemySlime [%u]: entity = [%u]", slime, data->entity);

    return slime;
}

void enemy_slime_update() {
    for (size_t i = 0; i < enemy_slime_pool.maxi; ++i) {
        if (!pool_alive(&enemy_slime_pool, i)) continue;

        EnemySlime *slime = pool_ptr(&enemy_slime_pool, i);

        Entity *slime_ett = entity_ptr(slime->entity);
        Vec2 *slime_pos = &slime_ett->pos;
        Vec2 target_pos = entity_ptr(slime->target)->pos;

        Vec2 move_dir; vec2_sub(&move_dir, target_pos, *slime_pos);
        vec2_normalize(&move_dir);

        Vec2 move_delta; vec2_scale(&move_delta, move_dir, enemy_slime_move_speed);
        vec2_add(slime_pos, *slime_pos, move_delta);
    }
}

void enemy_slime_draw() {
    for (size_t i = 0; i < enemy_slime_pool.maxi; ++i) {
        if (!pool_alive(&enemy_slime_pool, i)) continue;
        EnemySlime *slime = pool_ptr(&enemy_slime_pool, i);

        Entity slime_ett = *entity_ptr(slime->entity);
        Vec2 slime_pos = slime_ett.pos;

        draw_sprite_wpos(enemy_slime_sprite, slime_pos, 0, (Vec2){.5, .5}, (Vec2){4, 4});
    }
}
