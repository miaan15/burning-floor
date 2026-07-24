#pragma once

#include "alloc/arena.h"
#include "time/timel.h"

typedef struct {
    Arena *arena;

    TimelA tl;
    size_t *spr;
    size_t len;
    size_t cap;

    bool running;

    size_t *spr_sink;
} Ani;

void ani_init(Ani *ani, Arena *arena, float *time_hook, size_t cap, bool loop);
void ani_destroy(Ani *ani);

size_t ani_add(Ani *ani, float delta, size_t spr);
size_t ani_insert(Ani *ani, float time, size_t spr);

void ani_start(Ani *ani);
void ani_start_at(Ani *ani, float time);
void ani_end(Ani *ani);

void ani_update(Ani *ani);

void ani_sink_set(Ani *ani, size_t *spr_sink);
