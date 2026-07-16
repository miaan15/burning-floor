#include "enemy.h"
#include "game/player.h"
#include "global.h"
#include "log/log.h"
#include "spines/spines.h"
#include <math.h>
#include <stdalign.h>

EnemyMeleeDef enemy_melee_def = {0};

EnemyMng enemy_mng = {0};

void enemy_defs_init() {
    { // Melee
    spn_Mark cfgm_melee = spn_find(spn_root(&cfg_context), "asset/enemy");

    {
        spn_move(&cfgm_melee, "image");
        const char *image_path = spn_get_str(&cfgm_melee, 0);
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "%s/%s", ASSET_PATH, image_path);
        // enemy_melee_def.image =
        //     img_load_tex(&image_sys, full_path, SDL_SCALEMODE_NEAREST);
    }

    cfgm_melee = spn_find(spn_root(&cfg_context), "game/enemy/melee");

    spn_move(&cfgm_melee, "move_speed");
    enemy_melee_def.move_speed = spn_get_float(&cfgm_melee, 0);
    }
}

void _enemy_set_deleted(EnemyMng *mng, size_t index, bool v) {
    char *bitset = (char *)(mng->enemies_deleted);
    size_t byte_idx = index / 8;
    size_t bit_idx = index % 8;
    bitset[byte_idx] = (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
}
bool _enemy_is_deleted(EnemyMng *mng, size_t index) {
    char *bitset = (char *)(mng->enemies_deleted);
    return (bitset[index / 8] >> (index % 8)) & 1;
}

void enemy_mng_init(EnemyMng *mng, size_t enemies_cap) {
    ++enemies_cap; // for stub
    size_t arena_cap = 0;
    arena_cap += enemies_cap * sizeof(EnemyData);
    arena_cap += (enemies_cap + 7) / 8;
    arena_init(&mng->arena, arena_cap);

    mng->enemies_cap = enemies_cap;
    mng->enemies = (EnemyData *)arena_alloc(&mng->arena,
        enemies_cap * sizeof(EnemyData), alignof(EnemyData));
    mng->enemies_deleted = arena_alloc(&mng->arena, (enemies_cap + 7) / 8, 1);
    mng->enemies_offset = mng->enemies_count = mng->enemies_next_id = 0;

    // stub
    ++mng->enemies_offset; ++mng->enemies_count; ++mng->enemies_next_id;
    mng->enemies[0] = (EnemyData){0};
}

void enemy_mng_destroy(EnemyMng *mng) {
    arena_destroy(&mng->arena);
}

size_t enemy_make(EnemyMng *mng, uint8_t type) {
    if (mng->enemies_count >= mng->enemies_cap) {
        log_err("enemy_make(): full enemy capacity => Return 0");
        return 0;
    }

    size_t enemy = mng->enemies_next_id;
    if (enemy == mng->enemies_offset) {
        ++mng->enemies_offset;
        ++mng->enemies_next_id;
    } else {
        memcpy(&mng->enemies_next_id, &mng->enemies[enemy], sizeof(size_t));
    }

    _enemy_set_deleted(mng, enemy, 0);
    ++mng->enemies_count;

    memset(&mng->enemies[enemy], 0, sizeof(EnemyData));

    mng->enemies[enemy].type = type;

    log_debug("Made an enemy %zu: type %zu", enemy, type);

    return enemy;
}

void enemy_remove(EnemyMng *mng, size_t enemy) {
    if (enemy >= mng->enemies_offset) {
        log_err("img_remove_enemy(): index %zu out of bounds", enemy);
        return;
    }
    if (_enemy_is_deleted(mng, enemy)) {
        log_err("img_remove_enemy(): enemy %zu already deleted", enemy);
        return;
    }

    memcpy(&mng->enemies[enemy], &mng->enemies_next_id, sizeof(size_t));
    mng->enemies_next_id = enemy;

    _enemy_set_deleted(mng, enemy, 1);
    --mng->enemies_count;

    log_debug("Removed an enemy %zu", enemy);
}

void enemy_melee_init(EnemyMng *mng, size_t enemy) {
    EnemyData *enemy_data = &mng->enemies[enemy];
    if (enemy >= mng->enemies_offset || _enemy_is_deleted(mng, enemy)) {
        log_err("enemy_melee_init(): enemy %zu is not existed (deleted or out of bounds)", enemy);
        return;
    }
    if (enemy_data->type != ENEMY_MELEE) {
        log_err("enemy_melee_init(): enemy %zu is not melee", enemy);
        return;
    }
    EnemyMeleeData *melee_data = &enemy_data->melee_data;

    // melee_data->drawer = img_make_drawer(&image_sys, enemy_melee_def.image);

    melee_data->has_target = 1;
}

void enemy_melee_update_behavior(EnemyMng *mng, size_t enemy) {
    EnemyData *enemy_data = &mng->enemies[enemy];
    if (enemy >= mng->enemies_offset || _enemy_is_deleted(mng, enemy)) {
        log_err("enemy_melee_update_behavior(): enemy %zu is not existed (deleted or out of bounds)", enemy);
        return;
    }
    if (enemy_data->type != ENEMY_MELEE) {
        log_err("enemy_melee_update_behavior(): enemy %zu is not melee", enemy);
        return;
    }
    EnemyMeleeData *melee_data = &enemy_data->melee_data;

    if (!melee_data->has_target) {
        // TODO wander or something
        return;
    }

    melee_data->target = player_data.pos; // FIXME

    melee_data->move_dir.x = melee_data->target.x - melee_data->pos.x;
    melee_data->move_dir.y = melee_data->target.y - melee_data->pos.y;
    {
        float len = sqrtf((melee_data->move_dir.x * melee_data->move_dir.x) +
                          (melee_data->move_dir.y * melee_data->move_dir.y));
        const float EPSILON = 0.0001;
        if (len > EPSILON) {
            melee_data->move_dir.x /= len;
            melee_data->move_dir.y /= len;
        }
    }

    melee_data->pos.x += melee_data->move_dir.x * enemy_melee_def.move_speed;
    melee_data->pos.y += melee_data->move_dir.y * enemy_melee_def.move_speed;
}

void enemy_melee_update_render(EnemyMng *mng, size_t enemy) {
    EnemyData *enemy_data = &mng->enemies[enemy];
    if (enemy >= mng->enemies_offset || _enemy_is_deleted(mng, enemy)) {
        log_err("enemy_melee_update_render(): enemy %zu is not existed (deleted or out of bounds)", enemy);
        return;
    }
    if (enemy_data->type != ENEMY_MELEE) {
        log_err("enemy_melee_update_render(): enemy %zu is not melee", enemy);
        return;
    }
    EnemyMeleeData *melee_data = &enemy_data->melee_data;

    // img_get_drawer_ptr(&image_sys, melee_data->drawer)->srect = (Rect){0, 0, 20, 20};
    // img_feed_drawer_world(&image_sys, melee_data->drawer, melee_data->pos, 0, 1);
}

void enemy_melee_draw(EnemyMng *mng, size_t enemy) {
    EnemyData *enemy_data = &mng->enemies[enemy];
    if (enemy >= mng->enemies_offset || _enemy_is_deleted(mng, enemy)) {
        log_err("enemy_melee_draw(): enemy %zu is not existed (deleted or out of bounds)", enemy);
        return;
    }
    if (enemy_data->type != ENEMY_MELEE) {
        log_err("enemy_melee_draw(): enemy %zu is not melee", enemy);
        return;
    }
    EnemyMeleeData *melee_data = &enemy_data->melee_data;

    // img_draw(&image_sys, melee_data->drawer);
}
