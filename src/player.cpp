module;

export module player;

import common;
import context;
import system;
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

    size_t sprite_drawer = 0;
};

PlayerDef player_def{};
PlayerData player_data{};

void init_player() {
    // DEF
    // =========================================================================
    // FIXME
    player_def.move_speed = 10;

    // DATA
    // =========================================================================
    // FIXME
    player_data.pos = {0, 0};
    auto tex_path = asset_data_cxt.into("texture").get_as<std::string_view>(0);
    size_t tex = texture_sys.load(tex_path);
    auto sprite_group = asset_data_cxt.into("sprite/player");
    for (auto gr : sprite_group.group_range()) {
        auto it = gr.field_range().begin();
        size_t t = (*it++).as<size_t>();
        float x = (*it++).as<float>();
        float y = (*it++).as<float>();
        float w = (*it++).as<float>();
        float h = (*it++).as<float>();

        size_t sprite = sprite_sys.make_sprite(texture_sys.get(t),
                                               SDL_FRect{x, y, w, h});
    }
    player_data.sprite_drawer = sprite_sys.make_drawer(1);
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
    sprite_sys.update_drawer(player_data.sprite_drawer, player_data.pos);
}

void destroy_player() {
}

}
