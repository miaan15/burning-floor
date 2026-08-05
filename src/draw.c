#include "draw.h"
#include "context.h"
#include "log.h"
#include <assert.h>
#include <stdalign.h>

DrawSys draw_sys = {0};

Sprite *sprite_stub = NULL;
Drawer *drawer_stub = NULL;

void draw_init(DrawSys *sys, size_t sprite_cap, size_t drawer_cap, float zoom, float scale) {
    ++sprite_cap; ++drawer_cap;

    sys->sprite_cap = sprite_cap;
    sys->drawer_cap = drawer_cap;

    sys->sprites = arena_alloc(&global_ar, sprite_cap * sizeof(Sprite), alignof(Sprite));
    sys->sprites_len = 0;

    void *ptr = arena_alloc(&global_ar, pool_req_size(sizeof(Drawer), drawer_cap), alignof(max_align_t));
    pool_init_over(&sys->drawer_pool, ptr, sizeof(Drawer), drawer_cap);
    ptr = arena_alloc(&global_ar, pool_req_size(sizeof(DrawerHook), drawer_cap), alignof(max_align_t));
    pool_init_over(&sys->hook_pool, ptr, sizeof(DrawerHook), drawer_cap);

    sys->zoom = zoom;
    sys->scale = scale;

    // stub
    sys->sprites[sys->sprites_len++] = (Sprite){0}; // FIXME
    sprite_stub = &sys->sprites[0];

    Drawer stub_drawer = {&sys->sprites[0], {0, 0, 100, 100}, {0, 0}};
    drawer_stub = pool_new(&sys->drawer_pool, &stub_drawer).ptr;

    DrawerHook stub_hook = {0};
    pool_new(&sys->hook_pool, &stub_hook);
}

Sprite *draw_new_sprite(DrawSys *sys, SDL_Texture *tex, SDL_FRect *srect) {
    if (unlikely(sys->sprites_len >= sys->sprite_cap)) {
        log_err("draw_new_sprite(): too much sprite => stub");
        return sprite_stub;
    }
    Sprite *ptr = &sys->sprites[sys->sprites_len++];
    *ptr = (Sprite){tex, *srect};

    log_debug("New Sprite %p: tex: %p, srect: %.0f %.0f %.0f %.0f",
            ptr, tex, srect->x, srect->y, srect->w, srect->h);

    return ptr;
}

DrawerResult draw_new_drawer(DrawSys *sys, Sprite *sprite, SDL_FRect *drect) {
    Drawer drawer = {sprite != NULL ? sprite : drawer_stub->sprite,
                     drect != NULL ? *drect : drawer_stub->drect};
    PoolResult drawer_res = pool_new(&sys->drawer_pool, &drawer);
    PoolResult hook_res = pool_new(&sys->hook_pool, NULL);
    assert(drawer_res.meta == hook_res.meta);

    if (unlikely(drawer_res.ptr == NULL)) {
        log_err("draw_new_drawer(): too much drawer => stub");
        return (DrawerResult){ drawer_stub, NULL, 0 };
    }

    log_debug("New Drawer %p::%d: hook: %p, sprite: %p",
            drawer_res.ptr, drawer_res.meta, hook_res.ptr, sprite);

    return (DrawerResult){ drawer_res.ptr, hook_res.ptr, drawer_res.meta };
}

#define _draw_loop \
    size_t index = 0; \
    while (true) { \
        if (index >= sys->drawer_pool.maxi) break; \
        if (!(sys->drawer_pool.meta[index] & 1)) continue; \
        Drawer *drawer \
            = (Drawer *)((char *)sys->drawer_pool.raw + (index * sizeof(Drawer))); \
        DrawerHook *hook \
            = (DrawerHook *)((char *)sys->hook_pool.raw + (index * sizeof(DrawerHook))); \
        ++index; \

void draw_update(DrawSys *sys) {
    _draw_loop {
        if (tick_flag) {
            drawer->last_pos = (Vec2){drawer->drect.x, drawer->drect.y};
        }

        if (!hook->active) continue;

        Vec2 pos = hook->pos == NULL ? (Vec2){0, 0} : *hook->pos;
        Vec2 center = hook->center == NULL ? (Vec2){.5, .5} : *hook->center;

        SDL_FRect srect = drawer->sprite->srect;
        drawer->drect.x = (pos.x - (center.x * srect.w * sys->scale)) * sys->zoom;
        drawer->drect.y = window_h - ((pos.y + ((1 - center.y) * srect.h * sys->scale)) * sys->zoom);
        drawer->drect.w = srect.w * sys->scale * sys->zoom;
        drawer->drect.h = srect.h * sys->scale * sys->zoom;
    }}
}

void draw(DrawSys *sys) {
    _draw_loop {
        SDL_FRect drect;
        drect.x = drawer->last_pos.x + (drawer->drect.x - drawer->last_pos.x) * tick_alpha;
        drect.y = drawer->last_pos.y + (drawer->drect.y - drawer->last_pos.y) * tick_alpha;
        drect.w = drawer->drect.w;
        drect.h = drawer->drect.h;
        SDL_RenderTexture(renderer, drawer->sprite->tex, &drawer->sprite->srect, &drect);
    }}
}
