#include "draw.h"
#include "context.h"
#include "log.h"
#include <assert.h>
#include <stdalign.h>

// Texture
size_t texture_cap = 0;
SDL_Texture **textures = NULL;
size_t texture_len = 0;

SDL_Texture *texture_stub = NULL;

void texture_init(size_t cap) {
    ++cap;
    texture_cap = cap;
    textures = arena_alloc(&global_ar, cap * sizeof(SDL_Texture *), alignof(SDL_Texture *));
    texture_len = 0;

    // stub
    textures[texture_len++] = NULL; // FIXME create unique stub texture 32x32
    texture_stub = textures[0];
}

SDL_Texture *texture_new(const char *path, SDL_Renderer *renderer, SDL_ScaleMode scalemode) {
    if (unlikely(texture_len >= texture_cap)) {
        log_err("texture_new(): too much texture => stub");
        return texture_stub;
    }

    SDL_Surface *surf = SDL_LoadPNG(path);
    if (unlikely(!surf)) {
        log_err("texture_new(): try load surface from %s but %s => return stub",
                path, SDL_GetError());
        return texture_stub;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_SetTextureScaleMode(tex, scalemode);
    if (unlikely(!tex)) {
        log_err("texture_new(): try load texture from %s but %s => return stub",
                path, SDL_GetError());
        SDL_DestroySurface(surf);
        return texture_stub;
    }

    SDL_Texture **ptr = &textures[texture_len++];
    *ptr = tex;

    log_debug("New Texture [%zu][%p]: dir: %s, scale mode: %d; size: %d %d",
            ptr - textures, ptr, path, scalemode, tex->w, tex->h);

    SDL_DestroySurface(surf);

    return *ptr;
}

// Sprite
size_t sprite_cap = 0;
Sprite *sprites = NULL;
size_t sprite_len = 0;

Sprite *sprite_stub = NULL;

void sprite_init(size_t cap) {
    ++cap;
    sprite_cap = cap;
    sprites = arena_alloc(&global_ar, cap * sizeof(Sprite), alignof(Sprite));
    sprite_len = 0;

    // stub
    sprites[sprite_len++] = (Sprite){ texture_stub, (SDL_FRect){0, 0, 32, 32} };
    sprite_stub = &sprites[0];
}

Sprite *sprite_new(SDL_Texture *tex, SDL_FRect *srect) {
    if (unlikely(sprite_len >= sprite_cap)) {
        log_err("sprite_new(): too much sprite => stub");
        return sprite_stub;
    }

    Sprite *ptr = &sprites[sprite_len++];
    *ptr = (Sprite){ tex, *srect };

    log_debug("New Sprite [%zu][%p]: tex: [%p], srect: %.0f %.0f %.0f %.0f",
            (ptr - sprites), ptr, tex, srect->x, srect->y, srect->w, srect->h);

    return ptr;
}

// Draw
DrawSys draw_sys = {0};

Drawer *drawer_stub = NULL;
DrawerHook *drawer_hook_stub = NULL;

void draw_init(size_t cap, float zoom, float scale) {
    ++cap;
    draw_sys.cap = cap;
    draw_sys.zoom = zoom;
    draw_sys.scale = scale;

    { void *ptr = arena_alloc(&global_ar, pool_caps(sizeof(Drawer), cap), alignof(max_align_t));
    pool_init_over(&draw_sys.drawer_pool, ptr, sizeof(Drawer), cap); }

    { void *ptr = arena_alloc(&global_ar, pool_caps(sizeof(DrawerHook), cap), alignof(max_align_t));
    pool_init_over(&draw_sys.hook_pool, ptr, sizeof(DrawerHook), cap); }

    // stub
    Drawer d = { sprite_stub, {0, 0, 32, 32}, {0, 0} };
    pool_new(&draw_sys.drawer_pool, &d);
    pool_new(&draw_sys.hook_pool, NULL);

    drawer_stub = (Drawer *)pool_ptr(&draw_sys.drawer_pool, 0);
    drawer_hook_stub = (DrawerHook *)pool_ptr(&draw_sys.hook_pool, 0);
}

size_t draw_new(Sprite *sprite, SDL_FRect *scr_rect) {
    Drawer d = {sprite != NULL ? sprite : drawer_stub->sprite,
                scr_rect != NULL ? *scr_rect : drawer_stub->scr_rect};

    size_t di = pool_new(&draw_sys.drawer_pool, &d);
    size_t hi = pool_new(&draw_sys.hook_pool, NULL);

    if (unlikely(di == (size_t)-1)) {
        log_err("draw_new(): too much drawer => stub");
        return 0;
    }

    assert(di == hi);

    log_debug("New Drawer [%zu]: sprite: [%zu][%p]", di, sprite - sprites, sprite);

    return di;
}


Drawer *draw_drawer_ptr(size_t idx) {
    if (unlikely(!pool_alive(&draw_sys.drawer_pool, idx))) {
        log_err("draw_drawer_ptr(): try to call the dead with [%zu] => stub", idx);
        return drawer_stub;
    }
    return (Drawer *)pool_ptr(&draw_sys.drawer_pool, idx);
}

DrawerHook *draw_hook_ptr(size_t idx) {
    if (unlikely(!pool_alive(&draw_sys.hook_pool, idx))) {
        log_err("draw_hook_ptr(): try to call the dead with [%zu] => stub", idx);
        return drawer_hook_stub;
    }
    return (DrawerHook *)pool_ptr(&draw_sys.hook_pool, idx);
}

void draw_update() {
    for (size_t i = 0; i < draw_sys.drawer_pool.maxi; ++i) {
        if (!pool_alive(&draw_sys.drawer_pool, i)) continue;

        Drawer *drawer = pool_ptr(&draw_sys.drawer_pool, i);
        DrawerHook *hook = pool_ptr(&draw_sys.hook_pool, i);

        if (tick_flag) {
            drawer->last_pos = (Vec2){drawer->scr_rect.x, drawer->scr_rect.y};
        }

        if (!hook->active) continue;

        SDL_FRect srect = drawer->sprite->srect;

        Vec2 pos = hook->pos == NULL ? (Vec2){0, 0} : *hook->pos;
        Vec2 size = hook->size == NULL ? (Vec2){srect.w, srect.h} : *hook->size;
        Vec2 scale = hook->size == NULL ? (Vec2){1, 1} : *hook->scale;
        Vec2 center = hook->center == NULL ? (Vec2){.5, .5} : *hook->center;

        drawer->scr_rect.x = (pos.x - (center.x * size.x * scale.x * draw_sys.scale)) * draw_sys.zoom;
        drawer->scr_rect.y = (pos.y - (center.y * size.y * scale.y * draw_sys.scale)) * draw_sys.zoom;
        drawer->scr_rect.w = size.x * scale.x * draw_sys.scale * draw_sys.zoom;
        drawer->scr_rect.h = size.y * scale.y * draw_sys.scale * draw_sys.zoom;
    }
}

void draw() {
    for (size_t i = 0; i < draw_sys.drawer_pool.maxi; ++i) {
        if (!pool_alive(&draw_sys.drawer_pool, i)) continue;

        Drawer *drawer = pool_ptr(&draw_sys.drawer_pool, i);
        DrawerHook *hook = pool_ptr(&draw_sys.hook_pool, i);

        // FIXME for many reason, put all in a single buffer line first
        SDL_Texture *tex = drawer->sprite->tex;
        SDL_FRect srect = drawer->sprite->srect;
        SDL_FRect drect = (SDL_FRect){
            .x = drawer->last_pos.x + (drawer->scr_rect.x - drawer->last_pos.x) * tick_alpha,
            .y = window_h - (drawer->last_pos.y + (drawer->scr_rect.y - drawer->last_pos.y) * tick_alpha) - drawer->scr_rect.h,
            .w = drawer->scr_rect.w,
            .h = drawer->scr_rect.h,
        };

        SDL_RenderTexture(renderer, tex, &srect, &drect);
    }
}
