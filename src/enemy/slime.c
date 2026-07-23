#include "cglm/vec2.h"
#include "enemy.h"
#include "ett/ett.h"
#include "global.h"
#include "log/log.h"

void enemy_slime_init(EnemyMng *mng, Key enemy, vec2 pos) {
    EnemyIns *enemy_ins = enemy_get(mng, enemy);

    enemy_ins->type = ENEMY_SLIME;
    // enemy_ins->slime =
    EttIns *ett_ins = ett_get(&ett_mng, enemy_ins->ett);
    glm_vec2_copy(pos, ett_ins->pos);
    glm_vec2_copy(VEC2_HALF, ett_ins->rect_centr);
    // glm_vec2_copy(VEC2_ONE, ett_ins->rect_size);

    DrwrIns *drwr_ins = drwr_get(&drwr_mng, enemy_ins->drwr);
    drwr_set_spr(&drwr_mng, enemy_ins->drwr, enemy_mng.spr_slime[0]);
    drwr_hook_set_swpos(&drwr_mng, enemy_ins->drwr, ett_ins->pos, NULL, NULL);

    log_info("Init slime: enemy: %u.%u, pos: %.2f %.2f", enemy.idx, enemy.gen, pos[0], pos[1]);
}

void enemy_slime_update(EnemyMng *mng, Key enemy) {
    EnemyIns *enemy_ins = enemy_get(mng, enemy);
    EnemySlime *slime = &enemy_ins->slime; 

    EttIns *ett_ins = ett_get(&ett_mng, enemy_ins->ett);

    EttIns *target_ett_ins = ett_get(&ett_mng, enemy_ins->target);

    vec2 vel = {0};
    glm_vec2_sub(target_ett_ins->pos, ett_ins->pos, vel);
    glm_vec2_normalize_to(vel, vel);
    glm_vec2_mul(vel, (vec2){1, 1}, vel);
    glm_vec2_add(ett_ins->pos, vel, ett_ins->pos);
}
