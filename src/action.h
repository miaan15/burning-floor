#pragma once

#include "macro.h"

// can be optimize via SOA
typedef struct {
    u32 from, to, type;
    prx32 arg;
} action;

typedef enum {
    ACTION_HIT,
} action_type;

#define ACTION_EFF_OFFS 1

extern action *actions;
extern size_t actions_cap;
extern size_t actions_len;

void action_init(size_t scap);

size_t action_new(u32 from, u32 to, u32 type, const void *arg);

void action_update();
