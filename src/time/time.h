#pragma once

#include "alloc/arena.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float *time_ref;

    float *stamps;
    size_t cap;
    size_t len;

    float stamp_max;

    bool running;
    bool loop;

    size_t cur_stamp;

    float started_time;
    float paused_delta;
} Timeline;

void timeline_init(Timeline *tl, float *time, size_t cap, bool loop);
void timeline_destroy(Timeline *tl);

size_t timeline_add(Timeline *tl, float delta);
size_t timeline_insert(Timeline *tl, float time);

void timeline_start(Timeline *tl);
void timeline_pause(Timeline *tl);
void timeline_reset(Timeline *tl);

void timeline_set_loop(Timeline *tl, bool loop);

size_t timeline_cur_stamp(Timeline *tl);
size_t timeline_passed(Timeline *tl);

void timeline_skip(Timeline *tl, float delta);
void timeline_skip_to_stamp(Timeline *tl);

// =============================================================================
typedef struct {
    Arena *arena;

    float *time_ref;

    float *stamps;
    size_t cap;
    size_t len;

    float stamp_max;

    bool running;
    bool loop;

    size_t cur_stamp;

    float started_time;
    float paused_delta;
} TimelineA;

void timelinea_init(TimelineA *tl, Arena *arena, float *time, size_t cap, bool loop);

size_t timelinea_add(TimelineA *tl, float delta);
size_t timelinea_insert(TimelineA *tl, float time);

void timelinea_start(TimelineA *tl);
void timelinea_pause(TimelineA *tl);
void timelinea_reset(TimelineA *tl);

void timelinea_set_loop(TimelineA *tl, bool loop);

size_t timelinea_cur_stamp(TimelineA *tl);
size_t timelinea_passed(TimelineA *tl);

void timelinea_skip(TimelineA *tl, float delta);
void timelinea_skip_to_stamp(TimelineA *tl);

