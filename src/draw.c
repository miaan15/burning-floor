#include "draw.h"

#include "alloc/arena.h"
#include "context.h"
#include "draw_resource.h"
#include "log.h"
#include "macro.h"
#include <stdalign.h>

Drawer *drawers_raw = NULL;
DrawerMeta *drawer_metas_raw = NULL;
size_t drawers_len = 0;

RectDrawer *rect_drawers_raw = NULL;
RectDrawerMeta *rect_drawer_metas_raw = NULL;
size_t rect_drawers_len = 0;

void draw_init(size_t cap, size_t rect_cap) {
    drawers_raw =
        (Drawer *)arena_alloc(&global_ar, cap * sizeof(Drawer), alignof(Drawer));
    drawer_metas_raw =
        (DrawerMeta *)arena_alloc(&global_ar, cap * sizeof(DrawerMeta), alignof(DrawerMeta));
    drawers_len = 0;

    rect_drawers_raw =
        (RectDrawer *)arena_alloc(&global_ar, rect_cap * sizeof(RectDrawer), alignof(RectDrawer));
    rect_drawer_metas_raw =
        (RectDrawerMeta *)arena_alloc(&global_ar, rect_cap * sizeof(RectDrawerMeta), alignof(RectDrawerMeta));
    rect_drawers_len = 0;
}

void draw_sprite_wpos(uint32_t sprite, Vec2 pos, int8_t z, Vec2 center, Vec2 scale) {
    Drawer *drawer = &drawers_raw[drawers_len];
    DrawerMeta *meta = &drawer_metas_raw[drawers_len];
    ++drawers_len;

    size_t _sprite = sprite;
    if (unlikely(_sprite == 0 || _sprite >= sprites_len)) {
        log_warn("draw_sprite_wpos(): sprite is out of bounds => stub");
        _sprite = 0;
    }

    drawer->tex = sprites[_sprite].tex;
    drawer->srect = sprites[_sprite].srect;
    drawer->drect = (SDL_FRect) {
        .x = pos.x - (drawer->srect.w * scale.x * center.x),
        .y = window_h - (pos.y + (drawer->srect.h * scale.y * (1 - center.x))),
        .w = drawer->srect.w * scale.x,
        .h = drawer->srect.h * scale.y
    };
}

void draw_rect_wpos(Vec2 pos, Vec2 size, Vec2 center, u8 r, u8 g, u8 b, u8 a) {
    RectDrawer *drawer = &rect_drawers_raw[rect_drawers_len];
    RectDrawerMeta *meta = &rect_drawer_metas_raw[rect_drawers_len];
    ++rect_drawers_len;

    drawer->rect = (SDL_FRect) {
        .x = pos.x - (size.x * center.x),
        .y = window_h - (pos.y + (size.y * (1 - center.y))),
        .w = size.x,
        .h = size.y
    };
    drawer->r = r;
    drawer->g = g;
    drawer->b = b;
    drawer->a = a;
}

void draw() {
    // FIXME make the drawers sort by z -> tex -> ...
    for (size_t i = 0; i < drawers_len; ++i) {
        Drawer drawer = drawers_raw[i];
        SDL_RenderTexture(renderer, textures[(size_t)drawer.tex], &drawer.srect, &drawer.drect);
    }

    drawers_len = 0;

    for (size_t i = 0; i < rect_drawers_len; ++i) {
        RectDrawer rect_drawer = rect_drawers_raw[i];
        SDL_SetRenderDrawColor(renderer, rect_drawer.r, rect_drawer.g, rect_drawer.b, rect_drawer.a);
        SDL_RenderRect(renderer, &rect_drawer.rect);
    }

    rect_drawers_len = 0;
}
