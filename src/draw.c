#include "draw.h"
#include "context.h"
#include "log.h"
#include <assert.h>
#include <stdalign.h>

// Texture
size_t texture_cap = 0;
Texture *textures = NULL;
size_t texture_len = 0;

Texture *texture_stub = NULL;

void texture_init(size_t cap) {
    ++cap;
    texture_cap = cap;
    textures = arena_alloc(&global_ar, cap * sizeof(Texture), alignof(Texture));
    texture_len = 0;

    // stub
    textures[texture_len++] = (Texture){0}; // FIXME create unique stub texture
    texture_stub = &textures[0];
}

Texture *texture_new(const char *path, SDL_Renderer *renderer, SDL_ScaleMode scalemode) {
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

    Texture *ptr = &textures[texture_len++];
    *ptr = (Texture){ tex, surf->w, surf->h };


    log_debug("New Texture %p: dir: %s, scale mode: %d; size: %d %d",
            ptr, path, scalemode, surf->w, surf->h);

    SDL_DestroySurface(surf);

    return ptr;
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
    sprites[sprite_len++] = (Sprite){0}; // FIXME use should stub tex not null
    sprite_stub = &sprites[0];
}

Sprite *sprite_new(SDL_Texture *tex, SDL_FRect *srect) {
    if (unlikely(sprite_len >= sprite_cap)) {
        log_err("sprite_new(): too much sprite => stub");
        return sprite_stub;
    }

    Sprite *ptr = &sprites[sprite_len++];
    *ptr = (Sprite){ tex, *srect };

    log_debug("New Sprite %p: tex: %p, srect: %.0f %.0f %.0f %.0f",
            ptr, tex, srect->x, srect->y, srect->w, srect->h);

    return ptr;
}

// Draw
DrawSys draw_sys = {0};

Drawer *drawer_stub = NULL;
DrHook *drawer_hook_stub = NULL;

void draw_init(size_t cap, float zoom, float scale) {
    ++cap;
    draw_sys.cap = cap;
    draw_sys.zoom = zoom;
    draw_sys.scale = scale;

    { void *ptr = arena_alloc(&global_ar, pool_caps(sizeof(Drawer), cap), alignof(max_align_t));
    pool_init_over(&draw_sys.drawer_pool, ptr, sizeof(Drawer), cap); }

    { void *ptr = arena_alloc(&global_ar, pool_caps(sizeof(DrHook), cap), alignof(max_align_t));
    pool_init_over(&draw_sys.hook_pool, ptr, sizeof(DrHook), cap); }

    // stub
    Drawer d = { sprite_stub, {0, 0, 100, 100}, {0, 0} };
    drawer_stub = pool_new(&draw_sys.drawer_pool, &d);

    drawer_hook_stub = pool_new(&draw_sys.hook_pool, NULL);
}

DrawerAndHook draw_new(Sprite *sprite, SDL_FRect *drect) {
    Drawer d = {sprite != NULL ? sprite : drawer_stub->sprite,
                drect != NULL ? *drect : drawer_stub->drect};

    Drawer *dptr = pool_new(&draw_sys.drawer_pool, &d);
    DrHook *hptr = pool_new(&draw_sys.hook_pool, NULL);

    if (unlikely(dptr == NULL)) {
        log_err("draw_new(): too much drawer => stub");
        return (DrawerAndHook){ drawer_stub, drawer_hook_stub };
    }

    assert(pool_index(&draw_sys.drawer_pool, dptr) == pool_index(&draw_sys.hook_pool, hptr));

    log_debug("New Drawer %p - Hook %p: sprite: %p",
            dptr, hptr, sprite);

    return (DrawerAndHook){ dptr, hptr };
}

void draw_update() {
    for (size_t i = 0; i < draw_sys.drawer_pool.maxi; ++i) {
        if (!pool_alive_idx(&draw_sys.drawer_pool, i)) continue;

        Drawer *drawer = pool_ptr(&draw_sys.drawer_pool, i);
        DrHook *hook = pool_ptr(&draw_sys.hook_pool, i);

        if (tick_flag) {
            drawer->last_pos = (Vec2){drawer->drect.x, drawer->drect.y};
        }

        if (!hook->active) continue;

        Vec2 pos = hook->pos == NULL ? (Vec2){0, 0} : *hook->pos;
        Vec2 center = hook->center == NULL ? (Vec2){.5, .5} : *hook->center;

        SDL_FRect srect = drawer->sprite->srect;
        drawer->drect.x = (pos.x - (center.x * srect.w * draw_sys.scale)) * draw_sys.zoom;
        drawer->drect.y = window_h - ((pos.y + ((1 - center.y) * srect.h * draw_sys.scale)) * draw_sys.zoom);
        drawer->drect.w = srect.w * draw_sys.scale * draw_sys.zoom;
        drawer->drect.h = srect.h * draw_sys.scale * draw_sys.zoom;
    }
}

void draw() {
    for (size_t i = 0; i < draw_sys.drawer_pool.maxi; ++i) {
        if (!pool_alive_idx(&draw_sys.drawer_pool, i)) continue;

        Drawer *drawer = pool_ptr(&draw_sys.drawer_pool, i);
        DrHook *hook = pool_ptr(&draw_sys.hook_pool, i);

        SDL_FRect drect;
        drect.x = drawer->last_pos.x + (drawer->drect.x - drawer->last_pos.x) * tick_alpha;
        drect.y = drawer->last_pos.y + (drawer->drect.y - drawer->last_pos.y) * tick_alpha;
        drect.w = drawer->drect.w;
        drect.h = drawer->drect.h;
        SDL_RenderTexture(renderer, drawer->sprite->tex, &drawer->sprite->srect, &drect);
    }
}
