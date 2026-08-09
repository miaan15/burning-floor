#include "enemy.h"

#include "draw.h"
#include "entity.h"
#include "log.h"
#include <stdalign.h>
#include <context.h>

pool slime_pool = {0};

size_t slime_sprite = 0;
const float slime_move_speed = 1;

void slime_init(size_t cap) {
    size_t scap = pool_scap(sizeof(slime), cap);
    pool_init_in_arena(&slime_pool, &enemy_pools_ar, sizeof(slime), alignof(slime), scap);
    log_debug("New Slime Pool from %p to %p: cap = %zu",
            slime_pool.raw, (char *)slime_pool.raw + scap, cap);
}

size_t slime_new(slime *data) {
    size_t slime = pool_new(&slime_pool, data);
    if (slime == (size_t)-1) log_err("slime_new(): ");

    log_debug("Create Slime [%zu]: entity = [%zu]", slime, data->entity);

    return slime;
}

bool slime_remv(size_t idx) {
    if (!pool_alive(&slime_pool, idx)) {
        log_warn("slime_remv(): [idx] is already dead");
        return false;
    }
    slime *slime = pool_ptr(&slime_pool, idx);

    entity_remv(slime->entity);
    pool_remv(&slime_pool, idx);

    return true;
}

void slime_update() {
    for (size_t i = 0; i < slime_pool.len; ++i) {
        if (!pool_alive(&slime_pool, i)) continue;

        slime *slime = pool_ptr(&slime_pool, i);

        entity *slime_ett = entity_ptr(slime->entity);

        vec2 *slime_pos = &slime_ett->pos;
        vec2 target_pos = entity_ptr(slime->target)->pos;

        vec2 move_dir; vec2_sub(&move_dir, target_pos, *slime_pos);
        vec2_normalize(&move_dir);

        vec2 move_delta; vec2_scale(&move_delta, move_dir, slime_move_speed);
        vec2_add(slime_pos, *slime_pos, move_delta);

        if (slime_ett->health <= 0) slime_remv(i);
    }
}

void slime_draw() {
    for (size_t i = 0; i < slime_pool.len; ++i) {
        if (!pool_alive(&slime_pool, i)) continue;
        slime *slime = pool_ptr(&slime_pool, i);

        entity slime_ett = *entity_ptr(slime->entity);
        vec2 slime_pos = slime_ett.pos;

        draw_sprite_wpos(slime_sprite, slime_pos, 0, (vec2){.5, .5}, (vec2){4, 4});

        draw_rect_wpos((vec2){slime_pos.x, slime_pos.y + 50}, (vec2){slime_ett.health / 36 * 50, 10}, (vec2){.5, 0}, 255, 0, 0, 255);
    }
}

//
arena enemy_pools_ar = {0};

void enemy_init(size_t scap) {
    arena_init_in_arena(&enemy_pools_ar, &global_ar, scap);
    log_debug("New Enemy enemy_pools Arena from %p to %p: caps = %zu",
            enemy_pools_ar.raw, (char *)enemy_pools_ar.raw + enemy_pools_ar.scap, enemy_pools_ar.scap);
}
