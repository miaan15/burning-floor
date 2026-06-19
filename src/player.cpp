module;

export module player;

import common;
import context;

export namespace _ { 

struct PlayerDef {
    float move_speed;
};

struct PlayerData {
    vec2 pos{0, 0};

    vec2 move_dir{0, -1};
    vec2 facing_dir{0, -1};
};

SDL_Texture *player_tex = nullptr;

PlayerDef player_def{};
PlayerData player_data{};

void init_player() {
    // TEXTURE
    // =========================================================================
    const stdf::path player_tex_path = asset_path / "img/img_player.png";
    SDL_Surface *surf = SDL_LoadPNG(player_tex_path.c_str());

    player_tex = SDL_CreateTextureFromSurface(global_context.renderer, surf);
    SDL_SetTextureScaleMode(player_tex, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surf);

    if (!player_tex) {
        std::cerr << "Failed load surface: " << player_tex_path << "\n"
                  << "Error: " << SDL_GetError() << std::endl;
        return;
    }

    // DEF
    // =========================================================================
    // FIXME
    player_def.move_speed = 10;

    // DATA
    // =========================================================================
    // FIXME
    player_data.pos = {100, 100};
}

void logic_update_player() {
}

}
