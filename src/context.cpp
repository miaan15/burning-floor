module;

export module context;

import common;
import spines;

export int window_width = 0;
export int window_height = 0;
export SDL_Window *window = nullptr;
export SDL_Renderer *renderer = nullptr;

export int tps = 0;
export int pixel_size = 0;

export double cur_time_sec;
export double logic_update_alpha;

export spn_Context asset_data_cxt;
export spn_Context game_data_cxt;

export void init_context() {
    auto load_context = [](std::string_view path) {
        std::ifstream file(asset_path / path, std::ios::in | std::ios::binary);
        if (!file)
            std::cerr << "Could not open file: " << path << std::endl;

        return std::string((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
    };

    {
        auto str = load_context("data/data_asset.txt");
        spn_parse(&asset_data_cxt, str.c_str(), str.length());
    }
    {
        auto str = load_context("data/data_game.txt");
        spn_parse(&game_data_cxt, str.c_str(), str.length());
    }
}
