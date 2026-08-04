#include "draw.h"
#include "context.h"
#include "log.h"
#include <stdalign.h>

Sprite *sprite_list = NULL;

Pool drawer_pool = {0};
Pool drawer_hook_pool = {0};

float drawer_scale = 1;
float drawer_zoom = 1;

void draw_init(float zoom, float scale) {
    const size_t MAX_SPRITE = 1024;
    sprite_list = arena_alloc(&global_ar, MAX_SPRITE * sizeof(Sprite), alignof(Sprite));

    const size_t MAX_DRAWER = 1024;
    void *ptr = arena_alloc(&global_ar, pool_req_size(sizeof(Drawer), MAX_DRAWER), alignof(max_align_t));
    pool_init_over(&drawer_pool, ptr, sizeof(Drawer), MAX_DRAWER);
    ptr = arena_alloc(&global_ar, pool_req_size(sizeof(DrawerHook), MAX_DRAWER), alignof(max_align_t));
    pool_init_over(&drawer_hook_pool, ptr, sizeof(DrawerHook), MAX_DRAWER);

    drawer_zoom = zoom;
    drawer_scale = scale;
}

void draw_update_hook(DrawerHook *hook) {
    size_t offs = (char *)hook - (char *)drawer_hook_pool.raw;
    Drawer *drawer = (Drawer *)((char *)drawer_pool.raw + offs);

    Vec2 pos = hook->pos == NULL ? (Vec2){0, 0} : *hook->pos;
    Vec2 center = hook->center == NULL ? (Vec2){.5, .5} : *hook->center;

    SDL_FRect srect = drawer->sprite->srect;
    drawer->drect.x = (pos.x - (center.x * srect.w * drawer_scale)) * drawer_zoom;
    drawer->drect.y = window_h - ((pos.y + ((1 - center.y) * srect.h * drawer_scale)) * drawer_zoom);
    drawer->drect.w = srect.w * drawer_scale * drawer_zoom;
    drawer->drect.h = srect.h * drawer_scale * drawer_zoom;
}

void draw(Drawer *drawer) {
    SDL_RenderTexture(renderer, drawer->sprite->tex, &drawer->sprite->srect, &drawer->drect);
}
