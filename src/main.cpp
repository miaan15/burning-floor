import std;
import spines;

int main() {
    SpinesContext cxt;

    auto start_time = std::chrono::high_resolution_clock::now();

    auto err = cxt.parse(R"(
engine {
    name = "Cactus"
    version = 1

    memory {
        arena_size = 104857600
        pool_count = 4
        strict_alignment = 1,
        "debug_allocations"
    }

    ecs {
        max_entities = 65536
        max_components = 32
        signature_bitset_size = 64

        systems {
            "physics",
            "movement",
            "combat",
            "render"
        }
    }

    window {
        width = 1920
        height = 1080
        title = "Hellstone FPS"
        fullscreen = 1
        vsync = 0
    }
}

renderer {
    backend = "vulkan"
    max_fps = 144

    textures {
        anisotropy_level = 16,
        mipmapping = 1
    }

    lighting {
        shadow_map_resolution = 4096
        pcf_samples = 16
        volumetric_fog = 1,
        ambient_occlusion = 1
    }

    post_processing {
        bloom = 1
        bloom_intensity = 5,
        0,
        "color_grading_lut"
    }
}

gameplay {
    tick_rate = 60

    player {
        base_health = 100
        base_armor = 50
        movement {
            walk_speed = 400
            run_speed = 750
            dash_cooldown = 2
            jump_force = 15,
            1,
            "allow_bunny_hop"
        }
    }

    weapons {
        shotgun {
            damage_per_pellet = 8
            pellet_count = 12
            spread = 15
            ammo_capacity = 30,
            "close_range_only"
        }
        revolver {
            damage = 45
            headshot_multiplier = 3
            ammo_capacity = 6,
            "high_noon"
        }
    }

    ai {
        max_active_enemies = 120
        pathfinding = "navmesh"
        horde_mode = 1
        spawn_rate_multiplier = 2
    }
}

assets {
    base_path = "data/"

    models {
        "player.obj",
        "demon.obj",
        "health_pickup.obj",
        "ammo_box.obj"
    }

    sounds {
        "shoot.wav",
        "reload.wav",
        "enemy_growl.wav",
        "bgm_boss.ogg"
        volume_master = 8
        volume_sfx = 10
    }
}

levels {
    tutorial {
        id = 0,
        map = "maps/tuto.bsp",
        "safe_zone"
    }
    level_one {
        id = 1,
        map = "maps/e1m1.bsp",
        enemy_count = 50
    }
    boss_arena {
        id = 99,
        map = "maps/boss.bsp",
        boss_type = "hell_knight"
    }
}
    )");

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_ms = end_time - start_time;

    if (!err.type) {
        std::cout << "Time: " << elapsed_ms.count() << " ms.\n\n";
        cxt.print_debug();
    }
    else {
        std::cout << "Error parse: " << err.type << " at "
                  << err.line << ":" << err.column << std::endl;
    }

    cxt.destroy();
    return 0;
}
