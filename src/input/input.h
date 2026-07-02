#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "define.h"

extern const bool *cur_keyboard_state;
extern bool *last_keyboard_state;

void input_init();

void input_pump();

static inline void input_destroy() {
    if (last_keyboard_state) free(last_keyboard_state);
}

static inline bool is_key_down(Scancode scancode) {
    return cur_keyboard_state[(size_t)scancode]
           && !last_keyboard_state[(size_t)scancode];
}

static inline bool is_key_on(Scancode scancode) {
    return cur_keyboard_state[(size_t)scancode];
}

static inline bool is_key_up(Scancode scancode) {
    return !cur_keyboard_state[(size_t)scancode]
           && last_keyboard_state[(size_t)scancode];
}
