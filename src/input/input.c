#include "input.h"

#include <SDL3/SDL_keyboard.h>

const bool *cur_keyboard_state = NULL;
bool *last_keyboard_state = NULL;

int numkeys;

void input_init() {
    cur_keyboard_state = SDL_GetKeyboardState(&numkeys);
    last_keyboard_state = (bool *)malloc(numkeys * sizeof(bool));
}

void input_pump() {
    if (cur_keyboard_state && last_keyboard_state) {
        memcpy(last_keyboard_state, cur_keyboard_state, numkeys);
    }
    cur_keyboard_state = SDL_GetKeyboardState(&numkeys);
}
