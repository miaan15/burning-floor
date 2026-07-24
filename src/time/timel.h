#pragma once

#include "alloc/arena.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float *time_hook;

    float *stamps;
    size_t cap;
    size_t len;

    bool running;
    bool loop;

    size_t cur_stamp;

    float started_time;
    float paused_delta;
} Timel;

void timel_init(Timel *tl, float *time_hook, size_t cap, bool loop);
void timel_destroy(Timel *tl);

size_t timel_add(Timel *tl, float delta);
size_t timel_insert(Timel *tl, float time);

void timel_play(Timel *tl);
void timel_pause(Timel *tl);
void timel_reset(Timel *tl);

void timel_set_loop(Timel *tl, bool loop);

size_t timel_cur_stamp(Timel *tl);
size_t timel_passed(Timel *tl);

void timel_skip(Timel *tl, float delta);
void timel_skip_at(Timel *tl, float time);
void timel_to_stamp(Timel *tl, size_t stamp);

// =============================================================================
typedef struct {
    Arena *arena;

    float *time_hook;

    float *stamps;
    size_t cap;
    size_t len;

    bool running;
    bool loop;

    size_t cur_stamp;

    float started_time;
    float paused_delta;
} TimelA;

void timela_init(TimelA *tl, Arena *arena, float *time_hook, size_t cap, bool loop);
void timela_destroy(TimelA *tl);

size_t timela_add(TimelA *tl, float delta);
size_t timela_insert(TimelA *tl, float time);

void timela_play(TimelA *tl);
void timela_pause(TimelA *tl);
void timela_reset(TimelA *tl);

void timela_set_loop(TimelA *tl, bool loop);

size_t timela_cur_stamp(TimelA *tl);
size_t timela_passed(TimelA *tl);

void timela_skip(TimelA *tl, float delta);
void timela_skip_at(TimelA *tl, float time);
void timela_to_stamp(TimelA *tl, size_t stamp);
