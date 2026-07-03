#include "sprite.h"

#include "log/log.h"
#include "global.h"

void _set_deleted(SpriteSys *sys, size_t index, bool v) {
    char *bitset = (char *)(sys->drawers_deleted);
    size_t byte_idx = index / 8;
    size_t bit_idx = index % 8;
    bitset[byte_idx] = (bitset[byte_idx] & ~(1 << bit_idx)) | (v << bit_idx);
}
bool _is_deleted(SpriteSys *sys, size_t index) {
    char *bitset = (char *)(sys->drawers_deleted);
    return (bitset[index / 8] >> (index % 8)) & 1;
}

void sprite_sys_init(SpriteSys *sys, size_t sprites_cap, size_t drawers_cap) {
    sys->sprites_cap = sprites_cap + 1;
    sys->drawers_cap = drawers_cap + 1;

    size_t arena_cap = sprites_cap * sizeof(Sprite);
    arena_cap = align_up(arena_cap, alignof(SpriteDrawer));
    arena_cap += drawers_cap * sizeof(SpriteDrawer);
    arena_cap += (drawers_cap + 7) / 8;
    arena_init(&sys->arena, arena_cap);

    sys->sprites = (Sprite *)arena_alloc(
        &sys->arena, sprites_cap * sizeof(Sprite), alignof(Sprite));
    sys->sprites_len = 0;

    sys->drawers = (SpriteDrawer *)arena_alloc(
        &sys->arena, drawers_cap * sizeof(SpriteDrawer), alignof(SpriteDrawer));
    sys->drawers_offset = 0;
    sys->drawers_count = 0;
    sys->drawers_next_id = 0;
    sys->drawers_deleted = arena_alloc(&sys->arena, drawers_cap, 1);

    // stub // TODO proper stub
    ++sys->sprites_len;
    sys->sprites[0] = (Sprite){0};

    ++sys->drawers_offset;
    ++sys->drawers_count;
    ++sys->drawers_next_id;
    sys->drawers[0] = (SpriteDrawer){0};
}

void spr_sys_destroy(SpriteSys *sys) {
    arena_destroy(&sys->arena);
    sys->sprites = NULL;
    sys->drawers = NULL;
    sys->drawers_deleted = NULL;
    sys->sprites_len = sys->sprites_cap = sys->drawers_offset
        = sys->drawers_count = sys->drawers_cap = sys->drawers_next_id = 0;
}

size_t spr_make_sprite(SpriteSys *sys, SDL_Texture *texture, SDL_FRect *rect) {
    if (sys->sprites_len >= sys->sprites_cap) {
        log_warn("spr_make_sprite(): sprites are full => Return 0");
        return 0;
    }

    size_t index = sys->sprites_len++;
    sys->sprites[index].tex = texture;
    sys->sprites[index].rect = *rect;

    return index;
}

Sprite spr_get_sprite(SpriteSys *sys, size_t index) {
    if (index >= sys->sprites_len) {
        log_warn("spr_get_sprite(): index out of bounds => Return element-0");
        return sys->sprites[0];
    }
    return sys->sprites[index];
}

Sprite *spr_get_sprite_ptr(SpriteSys *sys, size_t index) {
    if (index >= sys->sprites_len) {
        log_warn("spr_get_sprite(): index out of bounds => Return element-0");
        return &sys->sprites[0];
    }
    return &sys->sprites[index];
}

size_t spr_make_drawer(SpriteSys *sys, size_t sprite) {
    if (sys->drawers_count >= sys->drawers_cap) {
        log_warn("spr_make_drawer(): drawers are full => Return 0");
        return 0;
    }

    size_t index = sys->drawers_next_id;
    if (index == sys->drawers_offset) {
        ++sys->drawers_offset;
        ++sys->drawers_next_id;
    } else {
        memcpy(&sys->drawers_next_id, &sys->drawers[index], sizeof(size_t));
    }

    _set_deleted(sys, index, 0);
    ++sys->drawers_count;

    memset(&sys->drawers[index], 0, sizeof(SpriteDrawer));

    return index;
}

void spr_remove_drawer(SpriteSys *sys, size_t index) {
    if (index >= sys->drawers_offset || _is_deleted(sys, index)) {
        log_warn("spr_remove_drawer(): already removed");
        return;
    }

    memcpy(&sys->drawers[index], &sys->drawers_next_id, sizeof(size_t));
    sys->drawers_next_id = index;

    _set_deleted(sys, index, 1);
    --sys->drawers_count;
}

SpriteDrawer spr_get_drawer(SpriteSys *sys, size_t index) {
    if (index >= sys->drawers_offset || _is_deleted(sys, index)) {
        log_warn("spr_get_drawer(): index out of bounds or deleted => Return element-0");
        return sys->drawers[0];
    }
    return sys->drawers[index];
}

SpriteDrawer *spr_get_drawer_ptr(SpriteSys *sys, size_t index) {
    if (index >= sys->drawers_offset || _is_deleted(sys, index)) {
        log_warn("spr_get_drawer_ptr(): index out of bounds or deleted => Return element-0");
        return &sys->drawers[0];
    }
    return &sys->drawers[index];
}

void spr_update_drawer(SpriteSys *sys, size_t index,
                       SDL_FPoint pos, bool flip, float scale) {
    if (index >= sys->drawers_offset || _is_deleted(sys, index)) {
        log_warn("spr_update_drawer(): index out of bounds or deleted");
        return;
    }

    SpriteDrawer *drawer = &sys->drawers[index];
    Sprite sprite = sys->sprites[drawer->sprite];

    drawer->drect.w = sprite.rect.w * pixel_size * scale;
    drawer->drect.h = sprite.rect.h * pixel_size * scale;

    drawer->drect.x = pos.x * pixel_size;
    drawer->drect.y = window_height
        - pos.y * pixel_size
        - drawer->drect.h;

    drawer->flip = flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
}

void spr_render(SpriteSys *sys, size_t index) {
    if (index >= sys->drawers_offset || _is_deleted(sys, index)) {
        log_warn("spr_update_drawer(): index out of bounds or deleted");
        return;
    }

    SpriteDrawer drawer = sys->drawers[index];
    Sprite sprite = sys->sprites[drawer.sprite];

    SDL_RenderTextureRotated(sdl_renderer, sprite.tex,
                             &sprite.rect, &drawer.drect, 0, NULL, drawer.flip);
}
