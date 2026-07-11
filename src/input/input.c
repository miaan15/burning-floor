#include "input.h"
#include "log/log.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_keyboard.h>

const bool *cur_keyboard_state = NULL;
bool *last_keyboard_state = NULL;

int numkeys;

void input_init() {
    cur_keyboard_state = SDL_GetKeyboardState(&numkeys);
    last_keyboard_state = (bool *)malloc(numkeys * sizeof(bool));
}

void input_update(bool *running) {
    if (!last_keyboard_state) {
        log_critical("oh no, bro you just forgot init input: input_init()");
    }

    if (cur_keyboard_state && last_keyboard_state) {
        memcpy(last_keyboard_state, cur_keyboard_state, numkeys * sizeof(bool));
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            *running = false;
        }
    }
}
