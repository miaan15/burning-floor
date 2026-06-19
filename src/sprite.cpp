module;

export module sprite;

import common;
import context;

export struct SpriteEntry {
    SDL_Texture *texture = nullptr;
    SDL_FRect srect{};
    SDL_FRect drect{};
    SDL_FlipMode flip = SDL_FlipMode::SDL_FLIP_NONE;
};

export SpriteEntry *sprite_list = nullptr;
export size_t sprite_list_len = 0;
export size_t sprite_list_cap = 0;

export void init_sprite_list(size_t cap) {
    sprite_list = (SpriteEntry *)std::malloc(cap * sizeof(SpriteEntry));
    sprite_list_cap = cap;
}

// export size_t make_sprite()
