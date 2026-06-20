module;

export module sprite;

import common;
import context;
import pool;

struct Sprite {
    SDL_Texture *tex = nullptr;
    SDL_FRect rect{};
};

struct SpriteDrawer {
    size_t sprite = 0;
    SDL_FRect drect{};
    SDL_FlipMode flip = SDL_FlipMode::SDL_FLIP_NONE;
};

export struct SpriteSys {
    constexpr static size_t arena_cap = 1024 * 1024; // 1MB
    Arena arena{};

    size_t sprite_cap = 0;
    size_t drawer_cap = 0;

    Sprite *sprites = nullptr;
    size_t sprites_len = 0;
    PoolWAlloc<SpriteDrawer, Arena> drawer_pool;

    void init(size_t sprite_cap = 256, size_t drawer_cap = -1) {
        this->sprite_cap = sprite_cap;
        this->drawer_cap = drawer_cap != -1 ? drawer_cap :sprite_cap * 16;

        arena.init(arena_cap);

        sprites = (Sprite *)arena.alloc(this->sprite_cap * sizeof(Sprite));
        drawer_pool.init(this->drawer_cap, &arena);

        ++sprites_len; // stub;
    }

    void destroy() {
        arena.destroy();
    }

    size_t make_sprite(SDL_Texture *tex, const SDL_FRect &rect) {
        if (sprites_len >= sprite_cap) return 0;
        size_t index = sprites_len++;
        sprites[index] = Sprite{tex, rect};
        return index;
    }

    Sprite get_sprite(size_t index) const { 
        if (index >= sprites_len) return Sprite{};
        return sprites[index];
    }
    Sprite *get_sprite_ptr(size_t index) {
        if (index >= sprites_len) return nullptr;
        return &sprites[index];
    }

    size_t make_drawer(size_t sprite) {
        return drawer_pool.add({.sprite = sprite});
    }

    void remove_drawer(size_t index) {
        drawer_pool.remove(index);
    }

    SpriteDrawer get_drawer(size_t index) const {
        return drawer_pool.get(index);
    }
    SpriteDrawer *get_drawer_ptr(size_t index) {
        return drawer_pool.get_ptr(index);
    }

    void update_drawer(size_t index, vec2 pos,
                       bool flip = false, float scale = 1) {
        if (!drawer_pool.check(index)) return;
        SpriteDrawer *drawer = drawer_pool.get_ptr(index);
        Sprite sprite = get_sprite(drawer->sprite);

        drawer->drect.w = sprite.rect.w * pixel_size * scale;
        drawer->drect.h = sprite.rect.h * pixel_size * scale;

        drawer->drect.x = pos.x * pixel_size;
        drawer->drect.y = window_height
            - pos.y * pixel_size
            - drawer->drect.h;

        drawer->flip = flip ? SDL_FlipMode::SDL_FLIP_HORIZONTAL
                                : SDL_FlipMode::SDL_FLIP_NONE;
    }

    void render_sprite(size_t index) {
        if (!drawer_pool.check(index)) return;
        SpriteDrawer drawer = drawer_pool.get(index);
        Sprite sprite = get_sprite(drawer.sprite);

        SDL_RenderTextureRotated(
            renderer,
            sprite.tex,
            &sprite.rect,
            &drawer.drect,
            0,
            nullptr,
            drawer.flip);
    }

    void render_all_sprites() {
        for (auto drawer : drawer_pool) {
            Sprite sprite = get_sprite(drawer.sprite);

            SDL_RenderTextureRotated(
                renderer,
                sprite.tex,
                &sprite.rect,
                &drawer.drect,
                0,
                nullptr,
                drawer.flip);
        }
    }
};
