#include "image.h"

#include <stdalign.h>
#include <stdlib.h>

#include "log/log.h"
#include "global.h"

void _drawer_set_deleted(ImageSys *sys, size_t index, bool v) {
    char *bitset = (char *)(sys->drawers_deleted);
    size_t byte_idx = index / 8;
    size_t bit_idx = index % 8;
    bitset[byte_idx] = (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
}
bool _drawer_is_deleted(ImageSys *sys, size_t index) {
    char *bitset = (char *)(sys->drawers_deleted);
    return (bitset[index / 8] >> (index % 8)) & 1;
}

void img_sys_init(ImageSys *sys, size_t images_cap, size_t drawers_cap) {
    ++images_cap; ++drawers_cap; // for stub

    size_t arena_cap = 0;
    arena_cap += images_cap * sizeof(SDL_Texture *);
    arena_cap = align_up(arena_cap, alignof(ImageTexData));
    arena_cap += images_cap * sizeof(ImageTexData);
    arena_cap = align_up(arena_cap, alignof(ImageDrawer));
    arena_cap += drawers_cap * sizeof(ImageDrawer);
    arena_cap += (drawers_cap + 7) / 8;
    arena_init(&sys->arena, arena_cap);


    sys->texs_cap = images_cap;
    sys->texs = (SDL_Texture **)arena_alloc(&sys->arena,
                images_cap * sizeof(SDL_Texture *), alignof(SDL_Texture *));
    sys->tex_datas = (ImageTexData *)arena_alloc(&sys->arena,
                     images_cap * sizeof(ImageTexData), alignof(ImageTexData));
    sys->texs_len = 0;

    sys->drawers_cap = drawers_cap;
    sys->drawers = (ImageDrawer *)arena_alloc(&sys->arena,
                    drawers_cap * sizeof(ImageDrawer), alignof(ImageDrawer));
    sys->drawers_deleted = arena_alloc(&sys->arena, (drawers_cap + 7) / 8, 1);
    sys->drawers_offset = sys->drawers_count = sys->drawers_next_id = 0;

    // stub // TODO proper stub
    ++sys->texs_len;
    sys->texs[0] = NULL; // FIXME
    ++sys->drawers_offset; ++sys->drawers_count; ++sys->drawers_next_id;
    sys->drawers[0] = (ImageDrawer){0}; // FIXME
}

void img_sys_destroy(ImageSys *sys) {
    for (size_t i = 0; i < sys->texs_len; ++i) {
        SDL_DestroyTexture(sys->texs[i]);
    }

    arena_destroy(&sys->arena);

    sys->texs_len = sys->texs_cap = 0;
    sys->drawers_cap = sys->drawers_offset = sys->drawers_count = sys->drawers_next_id = 0;
}

size_t img_load_tex(ImageSys *sys, const char *path, SDL_ScaleMode scalemode) {
    if (sys->texs_len >= sys->texs_cap) {
        log_err("img_load_tex(): full texture capacity => Return 0");
        return 0;
    }

    SDL_Surface *surf = SDL_LoadPNG(path);
    if (!surf) {
        log_err("img_load_tex(): try load surface from %s, %s => Return 0",
                path, SDL_GetError());
        return 0;
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(sdl_renderer, surf);
    SDL_SetTextureScaleMode(tex, scalemode);
    if (!tex) {
        log_err("img_load_tex(): try load texture from %s, %s => Return 0",
                path, SDL_GetError());
        SDL_DestroySurface(surf);
        return 0;
    }

    sys->texs[sys->texs_len] = tex;
    sys->tex_datas[sys->texs_len] = (ImageTexData){ surf->w, surf->h };

    log_debug("Loaded image %zu: dir: %s, scale mode: %d; size: %dx%d",
              sys->texs_len, path, scalemode, surf->w, surf->h);

    SDL_DestroySurface(surf);

    return sys->texs_len++;
}

SDL_Texture *img_get_tex(ImageSys *sys, size_t tex) {
    if (tex > sys->texs_len) {
        log_err("img_get_tex(): index %zu out of bounds => Return stub", tex);
        return sys->texs[0];
    }
    return sys->texs[tex];
}

size_t img_make_drawer(ImageSys *sys, size_t image) {
    if (sys->drawers_count >= sys->drawers_cap) {
        log_err("img_make_drawer(): full drawer capacity => Return 0");
        return 0;
    }

    size_t drawer = sys->drawers_next_id;
    if (drawer == sys->drawers_offset) {
        ++sys->drawers_offset;
        ++sys->drawers_next_id;
    } else {
        memcpy(&sys->drawers_next_id, &sys->drawers[drawer], sizeof(size_t));
    }

    _drawer_set_deleted(sys, drawer, 0);
    ++sys->drawers_count;

    memset(&sys->drawers[drawer], 0, sizeof(ImageDrawer));

    ImageTexData img_data = sys->tex_datas[image];
    sys->drawers[drawer].tex = img_get_tex(sys, image);
    sys->drawers[drawer].srect = (SDL_FRect){0, 0, img_data.w, img_data.h};

    log_debug("Made image drawer %zu: from image %zu", drawer, image);

    return drawer;
}
void img_remove_drawer(ImageSys *sys, size_t drawer) {
    if (drawer >= sys->drawers_offset || _drawer_is_deleted(sys, drawer)) {
        log_err("img_remove_drawer(): drawer %zu already deleted", drawer);
        return;
    }

    memcpy(&sys->drawers[drawer], &sys->drawers_next_id, sizeof(size_t));
    sys->drawers_next_id = drawer;

    _drawer_set_deleted(sys, drawer, 1);
    --sys->drawers_count;

    log_debug("Removed image drawer %zu", drawer);
}

ImageDrawer *img_get_drawer_ptr(ImageSys *sys, size_t drawer) {
    if (drawer >= sys->drawers_offset) {
        log_err("spr_get_drawer_ptr(): index %zu out of bounds => Return stub",
                drawer);
        return &sys->drawers[0];
    }
    if (_drawer_is_deleted(sys, drawer)) {
        log_err("spr_get_drawer_ptr(): drawer %zu deleted => Return stub",
                drawer);
        return &sys->drawers[0];
    }
    return &sys->drawers[drawer];
}

void img_feed_drawer_world(ImageSys *sys, size_t drawer,
                           SDL_FPoint pos, bool flip, float scale) {
    if (drawer >= sys->drawers_offset) {
        log_err("img_feed_drawer_world(): index %zu out of bounds", drawer);
        return;
    }
    if (_drawer_is_deleted(sys, drawer)) {
        log_err("img_feed_drawer_world(): drawer %zu deleted", drawer);
        return;
    }

    ImageDrawer *ins = &sys->drawers[drawer];

    ins->drect.w = ins->srect.w * pixel_size * scale;
    ins->drect.h = ins->srect.h * pixel_size * scale;

    ins->drect.x = pos.x * pixel_size;
    ins->drect.y = window_height
                   - pos.y * pixel_size
                   - ins->drect.h;

    ins->flip = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
}

void img_draw(ImageSys *sys, size_t drawer) {
    if (drawer >= sys->drawers_offset) {
        log_err("img_draw(): index %zu out of bounds", drawer);
        return;
    }
    if (_drawer_is_deleted(sys, drawer)) {
        log_err("img_draw(): drawer %zu deleted", drawer);
        return;
    }

    ImageDrawer ins = sys->drawers[drawer];

    SDL_RenderTextureRotated(sdl_renderer, ins.tex,
                             &ins.srect, &ins.drect, 0, NULL, ins.flip);
}
