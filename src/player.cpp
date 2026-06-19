module;

export module player;

import common;
import context;
import input;

export namespace _ {

struct PlayerSprite {
    SDL_Texture *texture = nullptr;
    SDL_FRect src_rect{};
    SDL_FRect dest_rect{};
    SDL_FlipMode flip = SDL_FlipMode::SDL_FLIP_NONE;
};

struct PlayerDef {
    float move_speed;
};

struct PlayerData {
    vec2 pos{0, 0};

    vec2 move_input{0, 0};
    vec2 move_dir{0, -1};
    vec2 facing_dir{0, -1};

    PlayerSprite sprite{};
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
    player_data.pos = {0, 0};
    player_data.sprite.texture = player_tex;
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
    player_data.sprite.texture = player_tex;

    player_data.sprite.src_rect = {0, 0, 32, 32};

    player_data.sprite.dest_rect.w =
        player_data.sprite.src_rect.w * global_context.pixel_size;
    player_data.sprite.dest_rect.h =
        player_data.sprite.src_rect.h * global_context.pixel_size;
    player_data.sprite.dest_rect.x =
        player_data.pos.x * global_context.pixel_size;
    player_data.sprite.dest_rect.y =
        global_context.window_height
        - player_data.pos.y * global_context.pixel_size
        - player_data.sprite.dest_rect.h;

    player_data.sprite.flip = SDL_FlipMode::SDL_FLIP_NONE;

    std::cout
        << player_data.sprite.dest_rect.x << " "
        << player_data.sprite.dest_rect.y << " "
        << player_data.sprite.dest_rect.w << " "
        << player_data.sprite.dest_rect.h << " "
        << "\n";
}

void destroy_player() {
    SDL_DestroyTexture(player_tex);
}

}
