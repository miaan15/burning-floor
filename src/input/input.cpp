module;

export module input;

export import :define;

import common;

export const bool *cur_keyboard_state;
export bool *last_keyboard_state;

export int numkeys;

export void init_input() {
    cur_keyboard_state = SDL_GetKeyboardState(&numkeys);
    last_keyboard_state = (bool *)std::malloc(numkeys * sizeof(bool));
}

export void roll_input() {
    if (cur_keyboard_state && last_keyboard_state) {
        std::memcpy(last_keyboard_state, cur_keyboard_state, numkeys);
    }

    SDL_PumpEvents();

    cur_keyboard_state = SDL_GetKeyboardState(&numkeys);
}

export bool is_key_trigger(Scancode scancode) {
    return cur_keyboard_state[(size_t)scancode]
           && !last_keyboard_state[(size_t)scancode];
}

export bool is_key(Scancode scancode) {
    return cur_keyboard_state[(size_t)scancode];
}

export bool is_key_release(Scancode scancode) {
    return !cur_keyboard_state[(size_t)scancode]
           && last_keyboard_state[(size_t)scancode];
}

export void destroy_input() {
    if (last_keyboard_state) std::free(last_keyboard_state);
}
