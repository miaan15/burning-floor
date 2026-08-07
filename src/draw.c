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

void draw_init(size_t cap) {
    drawers_raw =
        (Drawer *)arena_alloc(&global_ar, cap * sizeof(Drawer), alignof(Drawer));
    drawer_metas_raw =
        (DrawerMeta *)arena_alloc(&global_ar, cap * sizeof(DrawerMeta), alignof(DrawerMeta));
    drawers_len = 0;
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

void draw() {
    // FIXME make the drawers sort by z -> tex -> ...
    for (size_t i = 0; i < drawers_len; ++i) {
        Drawer drawer = drawers_raw[i];
        SDL_RenderTexture(renderer, textures[(size_t)drawer.tex], &drawer.srect, &drawer.drect);
    }

    drawers_len = 0;
}
