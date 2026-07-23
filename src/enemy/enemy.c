#include "enemy.h"
#include "cglm/vec2.h"
#include "ett/ett.h"
#include "global.h"
#include "log/log.h"

EnemyMng enemy_mng = {0};

void enemy_mng_init(EnemyMng *mng, size_t cap) {
    log_debug("Start init enemies");

    spn_Mark cfgm_pl = spn_find(spn_root(&cfg_context), "asset/enemy");

    {
    spn_move(&cfgm_pl, "image");
    const char *image_path = spn_get_str(&cfgm_pl, 0);
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", ASSET_PATH, image_path);

    mng->img = img_new(&img_mng, full_path, sdl_renderer, SDL_SCALEMODE_NEAREST);
    }

    for (size_t i = 0; i < 5; ++i)
        mng->spr_slime[i] = spr_new(&spr_mng, mng->img, (mat2){ 0 + 20 * i, 0, 20, 20 });
    for (size_t i = 0; i < 3; ++i)
        mng->spr_rock[i] = spr_new(&spr_mng, mng->img, (mat2){ 100 + 20 * i, 0, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        mng->spr_fly[i] = spr_new(&spr_mng, mng->img, (mat2){ 160 + 20 * i, 0, 20, 20 });

    ++cap;

    size_t arena_size = 0;
    arena_size += pool_msize(sizeof(EnemyIns), cap);
    arena_init(&mng->arena, arena_size);

    poola_init(&mng->enemy_pool, &mng->arena, sizeof(EnemyIns), alignof(EnemyIns), cap);

    // stub
    Key stub = poola_new(&mng->enemy_pool);
    *(EnemyIns *)poola_get(&mng->enemy_pool, stub) = (EnemyIns){0};
}

void enemy_mng_destroy(EnemyMng *mng) {
    poola_destroy(&mng->enemy_pool);
    arena_destroy(&mng->arena);
}

Key enemy_new(EnemyMng *mng) {
    assert(mng->arena.buffer);
    Key enemy = poola_new(&mng->enemy_pool);

    log_debug("Making a new enemy %u.%u", enemy.idx, enemy.gen);

    EnemyIns *enemy_ins = (EnemyIns *)poola_get(&mng->enemy_pool, enemy);
    vec2 vec_zero; glm_vec2_zero(vec_zero);
    enemy_ins->ett = ett_new(&ett_mng, VEC2_ZERO, VEC2_ZERO, VEC2_ZERO, VEC2_ZERO, 1);
    enemy_ins->drwr = drwr_new(&drwr_mng, 0, 0);

    return enemy;
}

void enemy_remv(EnemyMng *mng, Key enemy) {
    assert(mng->arena.buffer);
    if (!poola_alive(&mng->enemy_pool, enemy)) {
        log_err("enemy_remv(): enemy %u.%u is dead or invalid", enemy.idx, enemy.gen);
        return;
    }
    poola_remv(&mng->enemy_pool, enemy);
}

void enemy_init_as_slime(EnemyIns* enemy_ins, vec2 pos) {
    enemy_ins->type = ENEMY_SLIME;
    // enemy_ins->slime =
    EttIns *ett_ins = ett_get(&ett_mng, enemy_ins->ett);
    glm_vec2_copy(pos, ett_ins->pos);
    glm_vec2_copy(VEC2_HALF, ett_ins->rect_centr);
    // glm_vec2_copy(VEC2_ONE, ett_ins->rect_size);

    DrwrIns *drwr_ins = drwr_get(&drwr_mng, enemy_ins->drwr);
    drwr_set_spr(&drwr_mng, enemy_ins->drwr, enemy_mng.spr_slime[0]);
    drwr_hook_set_swpos(&drwr_mng, enemy_ins->drwr, ett_ins->pos, NULL, NULL);
}
