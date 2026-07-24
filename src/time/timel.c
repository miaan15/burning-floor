#include "timel.h"
#include "log/log.h"
#include "macro.h"
#include <assert.h>
#include <math.h>
#include <stdalign.h>
#include <stdlib.h>
#include <string.h>

void _timel_update(Timel *tl) {
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

void timel_init(Timel *tl, float *time, size_t cap, bool loop) {
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

void timel_destroy(Timel *tl) {
    free(tl->stamps);
    tl->time_hook = NULL;
    tl->stamps = NULL;
    tl->cap = tl->len = 0;
    tl->running = tl->loop = false;
    tl->cur_stamp = 0;
    tl->started_time = tl->paused_delta = 0;
}

size_t timel_add(Timel *tl, float delta) {
    if (tl->len >= tl->cap) {
        log_err("timel_add(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (delta <= 0) {
        log_err("timel_add(): delta should be > 0 (which was %f) => return 0", delta);
        return 0;
    }

    tl->stamps[tl->len] = tl->len == 0 ? delta
                                       : tl->stamps[tl->len - 1] + delta;
    return tl->len++;
}

size_t timel_insert(Timel *tl, float time) {
    if (tl->len >= tl->cap) {
        log_err("timel_insert(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (time <= 0) {
        log_err("timel_insert(): time should be > 0 (which was %f) => return 0", time);
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

void timel_play(Timel *tl) {
    if (unlikely(tl->running)) return;
    tl->running = true;
    tl->started_time = *tl->time_hook - tl->paused_delta;
}

void timel_pause(Timel *tl) {
    if (unlikely(!tl->running)) return;
    tl->running = false;
    tl->paused_delta = *tl->time_hook - tl->started_time;
}

void timel_reset(Timel *tl) {
    tl->started_time = *tl->time_hook;
    tl->paused_delta = 0;
    tl->cur_stamp = 0;
}

void timel_set_loop(Timel *tl, bool loop) {
    tl->loop = loop;
}

size_t timel_cur_stamp(Timel *tl) {
    _timel_update(tl);
    return tl->cur_stamp;
}

size_t timel_passed(Timel *tl) {
    _timel_update(tl);
    return tl->cur_stamp == 0 ? tl->len - 1 : tl->cur_stamp - 1;
}

void timel_skip(Timel *tl, float delta) {
    if (delta < 0) {
        log_err("timel_skip(): nuh uh, no skip backward");
        return;
    }
    tl->started_time -= delta;
    tl->paused_delta += delta;
}

void timel_skip_at(Timel *tl, float time) {
    if (time < 0) {
        log_err("timel_skip_at(): negative time?");
        return;
    }
    tl->started_time = *tl->time_hook - time;
    tl->paused_delta = time;
}

void timel_to_stamp(Timel *tl, size_t stamp) {
    if (unlikely(tl->cur_stamp == tl->len)) {
        log_err("timel_to_stamp(): stamp %zu out of bounds", stamp);
        return;
    }

    _timel_update(tl);
    tl->started_time = *tl->time_hook - tl->stamps[stamp];
    tl->paused_delta = tl->stamps[stamp];
}

// =============================================================================
void _timela_update(TimelA *tl) {
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

void timela_init(TimelA *tl, Arena *arena, float *time, size_t cap, bool loop) {
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

void timela_destroy(TimelA *tl) {
    tl->arena = NULL;
    tl->time_hook = NULL;
    tl->stamps = NULL;
    tl->cap = tl->len = 0;
    tl->running = tl->loop = false;
    tl->cur_stamp = 0;
    tl->started_time = tl->paused_delta = 0;
}

size_t timela_add(TimelA *tl, float delta) {
    if (tl->len >= tl->cap) {
        log_err("timela_add(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (delta <= 0) {
        log_err("timela_add(): delta should be > 0 (which was %f) => return 0", delta);
        return 0;
    }

    tl->stamps[tl->len] = tl->len == 0 ? delta
                                       : tl->stamps[tl->len - 1] + delta;
    return tl->len++;
}

size_t timela_insert(TimelA *tl, float time) {
    if (tl->len >= tl->cap) {
        log_err("timela_insert(): full capacity (which is %zu) => return 0", tl->cap);
        return 0;
    }
    if (time <= 0) {
        log_err("timela_insert(): time should be > 0 (which was %f) => return 0", time);
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

void timela_play(TimelA *tl) {
    if (unlikely(tl->running)) return;
    tl->running = true;
    tl->started_time = *tl->time_hook - tl->paused_delta;
}

void timela_pause(TimelA *tl) {
    if (unlikely(!tl->running)) return;
    tl->running = false;
    tl->paused_delta = *tl->time_hook - tl->started_time;
}

void timela_reset(TimelA *tl) {
    tl->started_time = *tl->time_hook;
    tl->paused_delta = 0;
    tl->cur_stamp = 0;
}

void timela_set_loop(TimelA *tl, bool loop) {
    tl->loop = loop;
}

size_t timela_cur_stamp(TimelA *tl) {
    _timela_update(tl);
    return tl->cur_stamp;
}

size_t timela_passed(TimelA *tl) {
    _timela_update(tl);
    return tl->cur_stamp == 0 ? tl->len - 1 : tl->cur_stamp - 1;
}

void timela_skip(TimelA *tl, float delta) {
    if (delta < 0) {
        log_err("timela_skip(): nuh uh, no skip backward");
        return;
    }
    tl->started_time -= delta;
    tl->paused_delta += delta;
}

void timela_skip_at(TimelA *tl, float time) {
    if (time < 0) {
        log_err("timel_skip_at(): negative time?");
        return;
    }
    tl->started_time = *tl->time_hook - time;
    tl->paused_delta = time;
}

void timela_to_stamp(TimelA *tl, size_t stamp) {
    if (unlikely(tl->cur_stamp == tl->len)) {
        log_err("timela_to_stamp(): stamp %zu out of bounds", stamp);
        return;
    }

    _timela_update(tl);
    tl->started_time = *tl->time_hook - tl->stamps[stamp];
    tl->paused_delta = tl->stamps[stamp];
}
