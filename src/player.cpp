module;

export module player;

import common;
import context;
import input;
import sprite;

export namespace _ {

struct PlayerDef {
    float move_speed;
};

struct PlayerData {
    vec2 pos{0, 0};

    vec2 move_input{0, 0};
    vec2 move_dir{0, -1};
    vec2 facing_dir{0, -1};

    size_t sprite_render = 0;
};

PlayerDef player_def{};
PlayerData player_data{};

void init_player() {
    // SPRITE
    // =========================================================================
    const stdf::path player_tex_path = asset_path / "img/img_player.png";
    SDL_Surface *surf = SDL_LoadPNG(player_tex_path.c_str());

    SDL_Texture *player_tex =
        SDL_CreateTextureFromSurface(global_context.renderer, surf);
    SDL_SetTextureScaleMode(player_tex, SDL_ScaleMode::SDL_SCALEMODE_NEAREST);
    SDL_DestroySurface(surf);
    if (!player_tex) {
        std::cerr << "Failed load surface: " << player_tex_path << "\n"
                  << "Error: " << SDL_GetError() << std::endl;
        return;
    }
    size_t tex = make_texture(player_tex);
    size_t sprite = make_sprite(tex, SDL_FRect{0, 0, 32, 32});

    // DEF
    // =========================================================================
    // FIXME
    player_def.move_speed = 10;

    // DATA
    // =========================================================================
    // FIXME
    player_data.pos = {0, 0};
    player_data.sprite_render = make_sprite_render(sprite);
}

void logic_update_player() {
    player_data.pos += player_data.move_dir * player_def.move_speed;
}

void frame_update_player() {
    player_data.move_input = {0, 0};
    if (is_key_on(SCANCODE_W)) player_data.move_input.y += 1;
    if (is_key_on(SCANCODE_A)) player_data.move_input.x -= 1;
    if (is_key_on(SCANCODE_S)) player_data.move_input.y -= 1;
    if (is_key_on(SCANCODE_D)) player_data.move_input.x += 1;

    constexpr float EPSILON = 0.00001;

    player_data.move_dir = length(player_data.move_input) > EPSILON ?
                               normalize(player_data.move_input)
                               : vec2{0, 0};

    player_data.facing_dir = player_data.move_dir;
    if (player_data.move_dir.x > EPSILON) player_data.facing_dir = { 1, 0};
    if (player_data.move_dir.x < EPSILON) player_data.facing_dir = {-1, 0};
}

void render_update_player() {
    update_sprite_render(player_data.sprite_render, player_data.pos);
}

void destroy_player() {
}

}
