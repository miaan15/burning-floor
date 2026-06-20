module;

export module sprite;

import common;
import context;
import pool;

export namespace _ {

struct Sprite {
    size_t tex = 0;
    SDL_FRect rect{};
};

struct SpriteRender {
    size_t sprite = 0;
    SDL_FRect drect{};
    SDL_FlipMode flip = SDL_FlipMode::SDL_FLIP_NONE;
};

constexpr size_t sprite_arena_size = 1024 * 1024; // 1MB
Arena sprite_arena{};

constexpr size_t max_texture = 32;
SDL_Texture **textures = nullptr;
size_t textures_len = 0;

constexpr size_t max_sprite_num = max_texture * 16;
PoolWAlloc<Sprite, Arena> sprite_pool{};

constexpr size_t max_sprite_render_num = max_sprite_num * 16;
PoolWAlloc<SpriteRender, Arena> sprite_render_pool{};

void init_sprite() {
    sprite_arena.init(sprite_arena_size);

    textures = (SDL_Texture **)sprite_arena.alloc(max_texture * sizeof(void *));
    textures_len = 0;

    sprite_pool.init(max_sprite_num, &sprite_arena);

    sprite_render_pool.init(max_sprite_render_num, &sprite_arena);

    // 0 default case
    sprite_pool.add({});
}

size_t make_texture(SDL_Texture *tex) {
    size_t index = textures_len++;
    if (index == max_texture) return 0;
    textures[index] = tex;
    return index;
}

size_t make_sprite(size_t tex, const SDL_FRect& rect) {
    return sprite_pool.add({.tex = tex, .rect = rect});
}

void destroy_sprite(size_t index) {
    sprite_pool.remove(index);
}

Sprite *get_sprite(size_t index) {
    return sprite_pool.get_ptr(index);
}

void update_sprite_render(size_t index, vec2 pos,
                          bool flip = false, float scale = 1) {
    SpriteRender *entry = sprite_render_pool.get_ptr(index);
    Sprite sprite = sprite_pool.get(entry->sprite);

    entry->drect.w = sprite.rect.w * global_context.pixel_size * scale;
    entry->drect.h = sprite.rect.h * global_context.pixel_size * scale;
    entry->drect.x = pos.x * global_context.pixel_size;
    entry->drect.y = global_context.window_height
                     - pos.y * global_context.pixel_size
                     - entry->drect.h;
    entry->flip = SDL_FlipMode::SDL_FLIP_NONE;
}

size_t make_sprite_render(size_t sprite_id, vec2 pos = vec2{0, 0},
                          bool flip = false, float scale = 1) {
    Sprite sprite = sprite_pool.get(sprite_id);
    size_t index = sprite_render_pool.add({.sprite = sprite_id});
    update_sprite_render(index, pos, flip, scale);
    return index;
}

SpriteRender *get_sprite_render(size_t index) {
    return sprite_render_pool.get_ptr(index);
}

void render_sprite(size_t index) {
    SpriteRender sprite_render = sprite_render_pool.get(index);
    Sprite sprite = sprite_pool.get(sprite_render.sprite);

    SDL_RenderTextureRotated(
        global_context.renderer,
        textures[sprite.tex],
        &sprite.rect,
        &sprite_render.drect,
        0,
        nullptr,
        sprite_render.flip);
}

void render_all_sprite() {
    for (auto sprite_render : sprite_render_pool) {
        Sprite sprite = sprite_pool.get(sprite_render.sprite);

        SDL_RenderTextureRotated(
            global_context.renderer,
            textures[sprite.tex],
            &sprite.rect,
            &sprite_render.drect,
            0,
            nullptr,
            sprite_render.flip);
    }
}

void destroy_all_sprite() {
    for (size_t i = 0; i < textures_len; ++i) {
        SDL_DestroyTexture(textures[i]);
    }

    sprite_pool.destroy();
    sprite_render_pool.destroy();
}

}
