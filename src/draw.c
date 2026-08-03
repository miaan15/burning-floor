#include "draw.h"
#include "context.h"
#include <stdalign.h>

Sprite *sprite_list = NULL;

Pool drawer_pool = {0};
Pool drawer_hook_pool = {0};

void draw_init() {
    const size_t MAX_SPRITE = 1024;
    sprite_list = arena_alloc(&global_ar, MAX_SPRITE * sizeof(Sprite), alignof(Sprite));

    const size_t MAX_DRAWER = 1024;
    void *ptr = arena_alloc(&global_ar, pool_req_size(sizeof(Drawer), MAX_DRAWER), alignof(max_align_t));
    pool_init_over(&drawer_pool, ptr, sizeof(Drawer), MAX_DRAWER);
    ptr = arena_alloc(&global_ar, pool_req_size(sizeof(DrawerHook), MAX_DRAWER), alignof(max_align_t));
    pool_init_over(&drawer_hook_pool, ptr, sizeof(DrawerHook), MAX_DRAWER);
}
