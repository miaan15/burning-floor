module;

export module texture;

import common;
import context;

export struct TextureSys {
    SDL_Texture **textures = nullptr;
    size_t len = 0;
    size_t cap = 0;
    stdf::path root_path;

    void init(const stdf::path &root_path, size_t cap = 64) {
        textures = (SDL_Texture **)std::malloc(cap * sizeof(SDL_Texture *));
        len = 0;
        this->cap = cap;
        this->root_path = root_path;
    }

    size_t load(stdf::path path) {
        if (len >= cap) return 0;

        path = root_path / path;
        SDL_Surface *surf = SDL_LoadPNG(path.c_str());
        if (!surf) {
            std::cerr << "Failed load surface: " << path << "\n"
                      << "Error: " << SDL_GetError() << std::endl;
            return 0;
        }
        SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_SetTextureScaleMode(tex, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);
        SDL_DestroySurface(surf);
        if (!tex) {
            std::cerr << "Failed load texture: " << path << "\n"
                      << "Error: " << SDL_GetError() << std::endl;
            return 0;
        }

        size_t index = len++;
        textures[index] = tex;
        return index;
    }

    void destroy() {
        for (size_t i = 0; i < len; ++i) {
            SDL_DestroyTexture(textures[i]);
        }
        std::free(textures);
    }

    SDL_Texture *get(size_t index) {
        if (index > len) return nullptr;
        return textures[index];
    }
};
