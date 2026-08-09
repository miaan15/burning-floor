#include "draw.h"

#include "alloc/arena.h"
#include "context.h"
#include "draw_resrc.h"
#include "log.h"
#include "macro.h"
#include <stdalign.h>

texdr *texdrs = NULL;
texdr_meta *texdr_metas = NULL;
size_t texdrs_cap = 0;
size_t texdrs_len = 0;

rectdr *rectdrs = NULL;
rectdr_meta *rectdr_metas = NULL;
size_t rectdrs_cap = 0;
size_t rectdrs_len = 0;

void draw_init(size_t texdr_cap, size_t rectdr_cap) {
    texdrs = (texdr *)arena_alloc(&global_ar, texdr_cap * sizeof(texdr), alignof(texdr));
    texdr_metas = (texdr_meta *)arena_alloc(&global_ar, texdr_cap * sizeof(texdr_meta), alignof(texdr_meta));;
    texdrs_cap = texdr_cap;
    texdrs_len = 0;

    rectdrs = (rectdr *)arena_alloc(&global_ar, rectdr_cap * sizeof(rectdr), alignof(rectdr));
    rectdr_metas = (rectdr_meta *)arena_alloc(&global_ar, rectdr_cap * sizeof(rectdr_meta), alignof(rectdr_meta));;
    rectdrs_cap = rectdr_cap;
    rectdrs_len = 0;
}

void draw_sprite_wpos(size_t sprite, vec2 pos, i8 z, vec2 center, vec2 scale) {
    texdr *ptr = &texdrs[texdrs_len];
    texdr_meta *meta = &texdr_metas[texdrs_len];
    ++texdrs_len;

    size_t spr = sprite;
    if (spr == 0 || spr >= sprites_len) {
        log_warn("draw_sprite_wpos(): sprite invalid => stub");
        spr = 0;
    }

    ptr->tex = sprites[spr].tex;
    ptr->srect = (SDL_FRect){ sprites[spr].srect.raw[0], sprites[spr].srect.raw[1],
                              sprites[spr].srect.raw[2], sprites[spr].srect.raw[3] };
    ptr->drect = (SDL_FRect) {
        .x = pos.x - (ptr->srect.w * scale.x * center.x),
        .y = window_h - (pos.y + (ptr->srect.h * scale.y * (1 - center.x))),
        .w = ptr->srect.w * scale.x,
        .h = ptr->srect.h * scale.y
    };
}

void draw_rect_wpos(vec2 pos, vec2 size, vec2 center, u8 r, u8 g, u8 b, u8 a) {
    rectdr *ptr = &rectdrs[rectdrs_len];
    rectdr_meta *meta = &rectdr_metas[rectdrs_len];
    ++rectdrs_len;

    ptr->rect = (SDL_FRect) {
        .x = pos.x - (size.x * center.x),
        .y = window_h - (pos.y + (size.y * (1 - center.y))),
        .w = size.x,
        .h = size.y
    };
    ptr->r = r;
    ptr->g = g;
    ptr->b = b;
    ptr->a = a;
}

void draw_present() {
    // FIXME make the drawers sort by z -> tex -> ...
    for (size_t i = 0; i < texdrs_len; ++i) {
        texdr tex_dr = texdrs[i];
        SDL_RenderTexture(renderer, textures[(size_t)tex_dr.tex], &tex_dr.srect, &tex_dr.drect);
    }

    texdrs_len = 0;

    for (size_t i = 0; i < rectdrs_len; ++i) {
        rectdr rect_dr = rectdrs[i];
        SDL_SetRenderDrawColor(renderer, rect_dr.r, rect_dr.g, rect_dr.b, rect_dr.a);
        SDL_RenderRect(renderer, &rect_dr.rect);
    }

    rectdrs_len = 0;
}
