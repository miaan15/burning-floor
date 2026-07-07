#include "enemy.h"
#include "log/log.h"
#include <stdalign.h>

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

void enemy_melee_update_behavior(EnemyMng *mng, size_t enemy) {}
