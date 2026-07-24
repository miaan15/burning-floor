#include "player.h"
#include "cglm/vec2.h"
#include "common.h"
#include "global.h"
#include "img/img.h"
#include "input/input.h"
#include "log/log.h"
#include "spines/spines.h"

PlayerDef player_def = {0};
Player player = {0};

void player_init() {
    log_debug("Start init player");

    memset(&player_def, 0, sizeof(PlayerDef));
    memset(&player, 0, sizeof(Player));

    const size_t ARENA_SIZE = 1 * 1024 * 1024; // 1 MB
    arena_init(&player.arena, ARENA_SIZE);

    spn_Mark cfgm_pl = spn_find(spn_root(&cfg_context), "asset/player");

    vec2 zero_vec2 = {0, 0};
    player.ett = ett_new(&ett_mng, zero_vec2, zero_vec2, zero_vec2, zero_vec2, 1);

    // Drwr
    {
    spn_move(&cfgm_pl, "image");
    const char *image_path = spn_get_str(&cfgm_pl, 0);
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", ASSET_PATH, image_path);

    player_def.img = img_new(&img_mng, full_path, sdl_renderer, SDL_SCALEMODE_NEAREST);
    }

    for (size_t i = 0; i < 4; ++i)
        player_def.spr_run_d[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 0 + 20 * i, 0, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_atk_d[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 80 + 20 * i, 0, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_roll_d[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 160 + 20 * i, 0, 20, 20 });

    for (size_t i = 0; i < 4; ++i)
        player_def.spr_run_u[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 0 + 20 * i, 20, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_atk_u[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 80 + 20 * i, 20, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_roll_u[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 160 + 20 * i, 20, 20, 20 });

    for (size_t i = 0; i < 4; ++i)
        player_def.spr_run_l[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 0 + 20 * i, 40, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_atk_l[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 80 + 20 * i, 40, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_roll_l[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 160 + 20 * i, 40, 20, 20 });

    for (size_t i = 0; i < 4; ++i)
        player_def.spr_run_r[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 0 + 20 * i, 60, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_atk_r[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 80 + 20 * i, 60, 20, 20 });
    for (size_t i = 0; i < 4; ++i)
        player_def.spr_roll_r[i] =
            spr_new(&spr_mng, player_def.img, (mat2){ 160 + 20 * i, 60, 20, 20 });

    player.drwr = drwr_new(&drwr_mng, player_def.spr_run_d[0], 0);

    EttIns *ett_ins = ett_get(&ett_mng, player.ett);
    drwr_hook_set_swpos(&drwr_mng, player.drwr, ett_ins->pos, NULL, NULL);

    // Ani - Timel
    cfgm_pl = spn_find(spn_root(&cfg_context), "ani/player");
    spn_step_flat(&cfgm_pl);

    timela_init(&player.ani_run_tl, &player.arena, &cur_frame_time, 4, true);
    spn_move_sibling(&cfgm_pl, "run");
    for (size_t i = 0; i < 4; ++i) {
        timela_add(&player.ani_run_tl, spn_get_float(&cfgm_pl, i));
        log_debug("player ani_run timel: stamp %zu at %.2f", i, player.ani_run_tl.stamps[i]);
    }
    timela_play(&player.ani_run_tl);

    timela_init(&player.ani_atk_tl, &player.arena, &cur_frame_time, 4, false);
    spn_move_sibling(&cfgm_pl, "atk");
    for (size_t i = 0; i < 4; ++i) {
        timela_add(&player.ani_atk_tl, spn_get_float(&cfgm_pl, i));
        log_debug("player ani_atk timel: stamp %zu at %.2f", i, player.ani_atk_tl.stamps[i]);
    }
    timela_play(&player.ani_atk_tl);

    timela_init(&player.ani_roll_tl, &player.arena, &cur_frame_time, 4, false);
    spn_move_sibling(&cfgm_pl, "roll");
    for (size_t i = 0; i < 4; ++i) {
        timela_add(&player.ani_roll_tl, spn_get_float(&cfgm_pl, i));
        log_debug("player ani_roll timel: stamp %zu at %.2f", i, player.ani_roll_tl.stamps[i]);
    }
    timela_play(&player.ani_roll_tl);

    // Params
    cfgm_pl = spn_find(spn_root(&cfg_context), "game/player");
    spn_step_flat(&cfgm_pl);

    spn_move_sibling(&cfgm_pl, "move_speed");
    player_def.move_speed = spn_get_float(&cfgm_pl, 0);

    spn_move_sibling(&cfgm_pl, "atk_dur");
    player_def.atk_dur = spn_get_float(&cfgm_pl, 0);

    spn_move_sibling(&cfgm_pl, "atk_cd");
    player_def.atk_cd = spn_get_float(&cfgm_pl, 0);

    log_info("Init player: move_speed = %.2f; atk_dur = %.2f; atk_cd = %.2f",
             player_def.move_speed, player_def.atk_dur, player_def.atk_cd);
}

void player_destroy() {
    if (key2u64(player.drwr)) drwr_remv(&drwr_mng, player.drwr);
    arena_destroy(&player.arena);
}

void player_logic_update() {
    // Input
    glm_vec2_copy(player.move_input, player.run_dir);
    if (glm_vec2_norm2(player.run_dir) > 0.00001) {
        if (fabs(player.run_dir[0]) > 0.00001) {
            player.face_dir[0] = player.run_dir[0] > 0 ? 1 : -1;;
            player.face_dir[1] = 0;
        } else {
            player.face_dir[0] = 0;
            player.face_dir[1] = player.run_dir[1] > 0 ? 1 : -1;
        }
    }

    player.atk_trggr = false;
    if (player.atk_able
        && (player.atk_button_inp || glm_vec2_norm2(player.atk_dir_inp) > 0.00001)) {
        player.atk_trggr = true;
        player.atk_able = false;

        if (fabs(player.atk_dir_inp[0]) > 0.00001) {
            player.atk_dir[0] = player.atk_dir_inp[0] > 0 ? 1 : -1;;
            player.atk_dir[1] = 0;
        } else if (fabs(player.atk_dir_inp[1]) > 0.00001) {
            player.atk_dir[0] = 0;
            player.atk_dir[1] = player.atk_dir_inp[1] > 0 ? 1 : -1;
        }
        else {
            glm_vec2_copy(player.face_dir, player.atk_dir);
        }

        player.atk_end_time = cur_logic_time + player_def.atk_dur;
        player.atk_off_cd_time = player.atk_end_time + player_def.atk_cd;
    }

    glm_vec2_zero(player.move_input);
    glm_vec2_zero(player.atk_dir_inp);
    player.atk_button_inp = false;

    // Actual
    EttIns *ett_ins = ett_get(&ett_mng, player.ett);

    player.atking = false;
    if (cur_logic_time <= player.atk_end_time) player.atking = true;

    if (player.atking) {
        glm_vec2_copy(player.atk_dir, player.face_dir);

        // TODO attack logic
    }

    if (cur_logic_time > player.atk_off_cd_time) {
        player.atk_able = true;
    }

    glm_vec2_muladd(player.run_dir,
                    (vec2){player_def.move_speed, player_def.move_speed},
                    ett_ins->pos);

    // Ani
    player.cur_ani = ANI_PLAYER_IDLE;
    if (glm_vec2_norm2(player.run_dir) > 0.00001) {
        player.cur_ani = ANI_PLAYER_RUN;
    }
    if (player.atking) player.cur_ani = ANI_PLAYER_ATK;
}

void player_frame_update() {
    if (is_key_on(SCANCODE_W) || is_key_on(SCANCODE_A) || is_key_on(SCANCODE_S) || is_key_on(SCANCODE_D)) {
        glm_vec2_zero(player.move_input);

        if (is_key_on(SCANCODE_W)) player.move_input[1] += 1;
        if (is_key_on(SCANCODE_A)) player.move_input[0] -= 1;
        if (is_key_on(SCANCODE_S)) player.move_input[1] -= 1;
        if (is_key_on(SCANCODE_D)) player.move_input[0] += 1;
        glm_vec2_normalize_to(player.move_input, player.move_input);
    }

    if (is_key_on(SCANCODE_UP) || is_key_on(SCANCODE_LEFT) || is_key_on(SCANCODE_DOWN) || is_key_on(SCANCODE_RIGHT)) {
        glm_vec2_zero(player.atk_dir_inp);

        if (is_key_on(SCANCODE_UP)) player.atk_dir_inp[1] += 1;
        if (is_key_on(SCANCODE_LEFT)) player.atk_dir_inp[0] -= 1;
        if (is_key_on(SCANCODE_DOWN)) player.atk_dir_inp[1] -= 1;
        if (is_key_on(SCANCODE_RIGHT)) player.atk_dir_inp[0] += 1;
        glm_vec2_normalize_to(player.atk_dir_inp, player.atk_dir_inp);
    }

    if (is_key_down(SCANCODE_J)) {
        player.atk_button_inp = true;
    }

    TimelA *cur_tl = NULL;
    switch (player.cur_ani) {
    case ANI_PLAYER_RUN: cur_tl = &player.ani_run_tl; break;
    case ANI_PLAYER_ATK: cur_tl = &player.ani_atk_tl; break;
    case ANI_PLAYER_ROLL: cur_tl = &player.ani_roll_tl; break;
    default: break;
    }
    if (player.last_ani != player.cur_ani)
        if (cur_tl) timela_reset(cur_tl);
    player.last_ani = player.cur_ani;

    switch (player.cur_ani) {
    case ANI_PLAYER_IDLE:
        assert(!cur_tl);
        if (player.face_dir[1] < 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_d[0]);
        else if (player.face_dir[1] > 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_u[0]);
        else if (player.face_dir[0] < 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_l[0]);
        else
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_r[0]);
    break;
    case ANI_PLAYER_RUN:
        assert(cur_tl);
        if (player.face_dir[1] < 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_d[timela_cur_stamp(cur_tl)]);
        else if (player.face_dir[1] > 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_u[timela_cur_stamp(cur_tl)]);
        else if (player.face_dir[0] < 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_l[timela_cur_stamp(cur_tl)]);
        else
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_run_r[timela_cur_stamp(cur_tl)]);
    break;
    case ANI_PLAYER_ATK:
        assert(cur_tl);
        if (player.face_dir[1] < 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_atk_d[timela_cur_stamp(cur_tl)]);
        else if (player.face_dir[1] > 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_atk_u[timela_cur_stamp(cur_tl)]);
        else if (player.face_dir[0] < 0)
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_atk_l[timela_cur_stamp(cur_tl)]);
        else
            drwr_set_spr(&drwr_mng, player.drwr, player_def.spr_atk_r[timela_cur_stamp(cur_tl)]);
    break;
    case ANI_PLAYER_ROLL: cur_tl = &player.ani_roll_tl; break;
    default: break;
    }
}
