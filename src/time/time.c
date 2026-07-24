#include "time.h"
#include "log/log.h"
#include "macro.h"
#include <assert.h>
#include <math.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

void _timeline_update(Timeline *tl) {
    if (!tl->running) return;

    float time = *tl->time_hook - tl->started_time;
    if (!tl->loop) {
        for (size_t i = tl->cur_stamp; i < tl->len; ++i) {
            if (time <= tl->stamps[i]) {
                tl->cur_stamp = i;
                return;
            }
        }
        tl->cur_stamp = tl->len;
        return;
    } else {
        time = fmodf(time, tl->stamps[tl->len - 1]);
        if (tl->cur_stamp != 0) {
            if (time < tl->stamps[tl->cur_stamp - 1]) {
                tl->cur_stamp = 0;
            }
        }
        for (size_t i = tl->cur_stamp; i < tl->len; ++i) {
            if (time <= tl->stamps[i]) {
                tl->cur_stamp = i;
                return;
            }
        }
        assert(false);
    }
}

void timeline_init(Timeline *tl, float *time, size_t cap, bool loop) {
    tl->time_hook = time;
    tl->stamps = (float *)malloc(cap * sizeof(float));
    tl->cap = cap;
    tl->len = 0;
    tl->running = false;
    tl->loop = loop;
    tl->cur_stamp = 0;
    tl->started_time = tl->paused_delta = 0;

    memset(tl->stamps, 0, cap * sizeof(float));
}

void timeline_destroy(Timeline *tl) {
    free(tl->stamps);
    tl->time_hook = NULL;
    tl->stamps = NULL;
    tl->cap = tl->len = 0;
    tl->running = tl->loop = false;
    tl->cur_stamp = 0;
    tl->started_time = tl->paused_delta = 0;
}

size_t timeline_add(Timeline *tl, float delta) {
    if (tl->len >= tl->cap) {
        log_err("timeline_add(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (delta <= 0) {
        log_err("timeline_add(): delta should be > 0 (which was %f) => return 0", delta);
        return 0;
    }

    tl->stamps[tl->len] = tl->len == 0 ? delta
                                       : tl->stamps[tl->len - 1] + delta;
    return tl->len++;
}

size_t timeline_insert(Timeline *tl, float time) {
    if (tl->len >= tl->cap) {
        log_err("timeline_insert(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (time <= 0) {
        log_err("timeline_insert(): time should be > 0 (which was %f) => return 0", time);
        return 0;
    }

    for (size_t i = 0; i <= tl->len; ++i) {
        if (tl->stamps[i] > time || i == tl->len) {
            for (size_t j = tl->len + 1; j-- > i + 1;) {
                tl->stamps[j] = tl->stamps[j - 1];
            }
            tl->stamps[i] = time;

            ++tl->len;
            return i;
        }
    }
    assert(false);
}

void timeline_play(Timeline *tl) {
    if (unlikely(tl->running)) return;
    tl->running = true;
    tl->started_time = *tl->time_hook - tl->paused_delta;
}

void timeline_pause(Timeline *tl) {
    if (unlikely(!tl->running)) return;
    tl->running = false;
    tl->paused_delta = *tl->time_hook - tl->started_time;
}

void timeline_reset(Timeline *tl) {
    tl->started_time = *tl->time_hook;
    tl->paused_delta = 0;
    tl->cur_stamp = 0;
}

void timeline_set_loop(Timeline *tl, bool loop) {
    tl->loop = loop;
}

size_t timeline_cur_stamp(Timeline *tl) {
    _timeline_update(tl);
    return tl->cur_stamp;
}

size_t timeline_passed(Timeline *tl) {
    _timeline_update(tl);
    return tl->cur_stamp == 0 ? tl->len - 1 : tl->cur_stamp - 1;
}

void timeline_skip(Timeline *tl, float delta) {
    if (delta < 0) {
        log_err("timeline_skip(): nuh uh, no skip backward");
        return;
    }
    tl->started_time -= delta;
    tl->paused_delta += delta;
}

void timeline_skip_at(Timeline *tl, float time) {
    if (time < 0) {
        log_err("timeline_skip_at(): negative time?");
        return;
    }
    tl->started_time = *tl->time_hook - time;
    tl->paused_delta = time;
}

void timeline_to_stamp(Timeline *tl, size_t stamp) {
    if (unlikely(tl->cur_stamp == tl->len)) {
        log_err("timeline_to_stamp(): stamp %zu out of bounds", stamp);
        return;
    }

    _timeline_update(tl);
    tl->started_time = *tl->time_hook - tl->stamps[stamp];
    tl->paused_delta = tl->stamps[stamp];
}

// =============================================================================
void _timelinea_update(TimelineA *tl) {
    if (!tl->running) return;

    float time = *tl->time_hook - tl->started_time;
    if (!tl->loop) {
        for (size_t i = tl->cur_stamp; i < tl->len; ++i) {
            if (time <= tl->stamps[i]) {
                tl->cur_stamp = i;
                return;
            }
        }
        tl->cur_stamp = tl->len;
        return;
    } else {
        time = fmodf(time, tl->stamps[tl->len - 1]);
        if (tl->cur_stamp != 0) {
            if (time < tl->stamps[tl->cur_stamp - 1]) {
                tl->cur_stamp = 0;
            }
        }
        for (size_t i = tl->cur_stamp; i < tl->len; ++i) {
            if (time <= tl->stamps[i]) {
                tl->cur_stamp = i;
                return;
            }
        }
        assert(false);
    }
}

void timelinea_init(TimelineA *tl, Arena *arena, float *time, size_t cap, bool loop) {
    tl->arena = arena;
    tl->time_hook = time;
    tl->stamps = (float *)arena_alloc(arena, cap * sizeof(float), alignof(float));
    tl->cap = cap;
    tl->len = 0;
    tl->running = false;
    tl->loop = loop;
    tl->cur_stamp = 0;
    tl->started_time = tl->paused_delta = 0;
}

void timelinea_destroy(TimelineA *tl) {
    tl->arena = NULL;
    tl->time_hook = NULL;
    tl->stamps = NULL;
    tl->cap = tl->len = 0;
    tl->running = tl->loop = false;
    tl->cur_stamp = 0;
    tl->started_time = tl->paused_delta = 0;
}

size_t timelinea_add(TimelineA *tl, float delta) {
    if (tl->len >= tl->cap) {
        log_err("timelinea_add(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (delta <= 0) {
        log_err("timelinea_add(): delta should be > 0 (which was %f) => return 0", delta);
        return 0;
    }

    tl->stamps[tl->len] = tl->len == 0 ? delta
                                       : tl->stamps[tl->len - 1] + delta;
    return tl->len++;
}

size_t timelinea_insert(TimelineA *tl, float time) {
    if (tl->len >= tl->cap) {
        log_err("timelinea_insert(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (time <= 0) {
        log_err("timelinea_insert(): time should be > 0 (which was %f) => return 0", time);
        return 0;
    }

    for (size_t i = 0; i <= tl->len; ++i) {
        if (tl->stamps[i] > time || i == tl->len) {
            for (size_t j = tl->len + 1; j-- > i + 1;) {
                tl->stamps[j] = tl->stamps[j - 1];
            }
            tl->stamps[i] = time;

            ++tl->len;
            return i;
        }
    }
    assert(false);
}

void timelinea_play(TimelineA *tl) {
    if (unlikely(tl->running)) return;
    tl->running = true;
    tl->started_time = *tl->time_hook - tl->paused_delta;
}

void timelinea_pause(TimelineA *tl) {
    if (unlikely(!tl->running)) return;
    tl->running = false;
    tl->paused_delta = *tl->time_hook - tl->started_time;
}

void timelinea_reset(TimelineA *tl) {
    tl->started_time = *tl->time_hook;
    tl->paused_delta = 0;
    tl->cur_stamp = 0;
}

void timelinea_set_loop(TimelineA *tl, bool loop) {
    tl->loop = loop;
}

size_t timelinea_cur_stamp(TimelineA *tl) {
    _timelinea_update(tl);
    return tl->cur_stamp;
}

size_t timelinea_passed(TimelineA *tl) {
    _timelinea_update(tl);
    return tl->cur_stamp == 0 ? tl->len - 1 : tl->cur_stamp - 1;
}

void timelinea_skip(TimelineA *tl, float delta) {
    if (delta < 0) {
        log_err("timelinea_skip(): nuh uh, no skip backward");
        return;
    }
    tl->started_time -= delta;
    tl->paused_delta += delta;
}

void timelinea_skip_at(TimelineA *tl, float time) {
    if (time < 0) {
        log_err("timeline_skip_at(): negative time?");
        return;
    }
    tl->started_time = *tl->time_hook - time;
    tl->paused_delta = time;
}

void timelinea_to_stamp(TimelineA *tl, size_t stamp) {
    if (unlikely(tl->cur_stamp == tl->len)) {
        log_err("timelinea_to_stamp(): stamp %zu out of bounds", stamp);
        return;
    }

    _timelinea_update(tl);
    tl->started_time = *tl->time_hook - tl->stamps[stamp];
    tl->paused_delta = tl->stamps[stamp];
}
