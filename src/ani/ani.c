#include "ani.h"
#include "log/log.h"
#include <assert.h>

void ani_init(Ani *ani, Arena *arena, float *time_hook, size_t cap, bool loop) {
    ani->arena = arena;
    timelinea_init(&ani->tl, arena, time_hook, cap, loop);
    ani->spr = (size_t *)arena_alloc(arena, cap * sizeof(size_t), alignof(size_t));
    ani->len = 0;
    ani->cap = cap;
    ani->running = false;
    ani->spr_sink = NULL;
}

void ani_destroy(Ani *ani) {
    ani->arena = NULL;
    timelinea_destroy(&ani->tl);
    ani->spr = NULL;
    ani->spr_sink = NULL;
    ani->len = ani->cap = 0;
}

size_t ani_add(Ani *ani, float delta, size_t spr) {
    if (ani->len >= ani->cap) {
        log_err("ani_add(): full capacity (which is %zu) => return 0", ani->cap);
        return 0;
    }
    if (delta <= 0) {
        log_err("ani_add(): delta should be > 0 (which was %f) => return 0", delta);
        return 0;
    }

    timelinea_add(&ani->tl, delta);
    ani->spr[ani->len] = spr;

    return ani->len++;
}

size_t ani_insert(Ani *ani, float time, size_t spr) {
    if (ani->len >= ani->cap) {
        log_err("ani_add(): full capacity (which is %zu) => return 0", ani->cap);
        return 0;
    }
    if (time <= 0) {
        log_err("ani_add(): time should be > 0 (which was %f) => return 0", time);
        return 0;
    }

    size_t i = timelinea_insert(&ani->tl, time);
    ani->spr[i] = spr;

    ++ani->len;

    return i;
}

void ani_start(Ani *ani) {
    timelinea_reset(&ani->tl);
    timelinea_play(&ani->tl);
    ani->running = true;
}

void ani_start_at(Ani *ani, float time) {
    if (time < 0) {
        log_err("ani_start_at(): time is not negetive");
        return;
    }
    timelinea_skip_at(&ani->tl, time);
    timelinea_play(&ani->tl);
    ani->running = true;
}

void ani_end(Ani *ani) {
    ani->running = false;
}

void ani_update(Ani *ani) {
    if (!ani->running) return;
    if (ani->spr_sink != NULL)
        *ani->spr_sink = ani->spr[timelinea_cur_stamp(&ani->tl)];
}

void ani_sink_set(Ani *ani, size_t *spr_sink) {
    ani->spr_sink = spr_sink;
}
