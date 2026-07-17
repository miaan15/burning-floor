// #include "player.h"
//
// #include "global.h"
// #include "input/input.h"
// #include "log/log.h"
// #include "spines/spines.h"
// #include <math.h>
//
// PlayerDef player_def = {0};
// PlayerData player_data = {0};
//
// void player_init() {
//     spn_Mark cfgm_pl = spn_find(spn_root(&cfg_context), "asset/player");
//
//     { //
//     spn_move(&cfgm_pl, "image");
//     const char *image_path = spn_get_str(&cfgm_pl, 0);
//     char full_path[256];
//     log_info("%s", image_path);
//     snprintf(full_path, sizeof(full_path), "%s/%s", ASSET_PATH, image_path);
//     // player_data.image =
//     //     img_load_tex(&image_sys, full_path, SDL_SCALEMODE_NEAREST);
//     // player_data.drawer = img_make_drawer(&image_sys, player_data.image);
//     }
//
//     cfgm_pl = spn_find(spn_root(&cfg_context), "game/player");
//
//     spn_move(&cfgm_pl, "move_speed");
//     player_def.move_speed = spn_get_float(&cfgm_pl, 0);
//
//     spn_move_sibling(&cfgm_pl, "move_ani_delta");
//     player_def.move_ani_delta = spn_get_float(&cfgm_pl, 0);
//
//     spn_move_sibling(&cfgm_pl, "attack_duration");
//     player_def.attack_duration = spn_get_float(&cfgm_pl, 0);
//
//     spn_move_sibling(&cfgm_pl, "attack_cooldown");
//     player_def.attack_cooldown = spn_get_float(&cfgm_pl, 0);
//
//     spn_move_sibling(&cfgm_pl, "attack_hold_ani_delta");
//     player_def.attack_hold_ani_delta = spn_get_float(&cfgm_pl, 0);
//
//     spn_move_sibling(&cfgm_pl, "attack_act_ani_delta");
//     player_def.attack_act_ani_delta = spn_get_float(&cfgm_pl, 0);
// }
//
// void player_logic_update() {
//     // Get from move input
//     const float EPSILON = 0.0001;
//     player_data.move_dir = player_data.move_input;
//     float move_input_len =
//         sqrtf((player_data.move_dir.x * player_data.move_dir.x) +
//               (player_data.move_dir.y * player_data.move_dir.y));
//     if (move_input_len > EPSILON) {
//         player_data.move_dir.x /= move_input_len;
//         player_data.move_dir.y /= move_input_len;
//     } else {
//         player_data.move_dir.x = 0;
//         player_data.move_dir.y = 0;
//     }
//
//     // Get from attack input
//     player_data.attack_trigger = 0;
//     if (player_data.can_attack) {
//         if (player_data.attack_input) {
//             player_data.attack_trigger = 1;
//             player_data.attack_dir = player_data.attack_dir_input;
//         }
//     }
//     player_data.attack_input = 0;
//     player_data.attack_dir_input = (Vec2int){0};
//
//     // Attack
//     if (player_data.attack_trigger) {
//         player_data.can_attack = 0;
//         player_data.attacking = 1;
//
//         player_data.attack_end_time = cur_logic_time + player_def.attack_duration;
//         player_data.attack_off_cd_time = cur_logic_time + player_def.attack_cooldown;
//     }
//
//     if (cur_logic_time > player_data.attack_end_time) {
//         player_data.attack_end_time = INFINITY;
//         player_data.attacking = 0;
//     }
//
//     if (player_data.attacking) {
//         // TODO attack logic
//
//         player_data.facing_dir.x = player_data.attack_dir.x;
//         player_data.facing_dir.y = player_data.attack_dir.y;
//     }
//
//     if (cur_logic_time > player_data.attack_off_cd_time) {
//         player_data.can_attack = 1;
//         player_data.attack_end_time = INFINITY;
//     }
//
//     // Move
//     if (!player_data.attacking) {
//         player_data.pos.x += player_data.move_dir.x * player_def.move_speed;
//         player_data.pos.y += player_data.move_dir.y * player_def.move_speed;
//     }
// }
//
// void player_frame_update() {
//     player_data.move_input = (Vec2){0};
//     if (is_key_on(SCANCODE_W)) player_data.move_input.y += 1;
//     if (is_key_on(SCANCODE_A)) player_data.move_input.x -= 1;
//     if (is_key_on(SCANCODE_S)) player_data.move_input.y -= 1;
//     if (is_key_on(SCANCODE_D)) player_data.move_input.x += 1;
//
//     const float EPSILON = 0.0001;
//     player_data.move_dir = player_data.move_input;
//     float move_input_len =
//         sqrtf((player_data.move_dir.x * player_data.move_dir.x) +
//               (player_data.move_dir.y * player_data.move_dir.y));
//     if (move_input_len > EPSILON) {
//         if (fabsf(player_data.move_input.x) > EPSILON) {
//             player_data.facing_dir.x = copysignf(1, player_data.move_input.x);
//             player_data.facing_dir.y = 0;
//         } else {
//             player_data.facing_dir.x = 0;
//             player_data.facing_dir.y = copysignf(1, player_data.move_input.y);
//         }
//     }
//
//     if (player_data.attack_dir_input.x + player_data.attack_dir_input.y == 0) {
//         if (is_key_down(SCANCODE_UP   )) player_data.attack_dir_input.y += 1;
//         if (is_key_down(SCANCODE_LEFT )) player_data.attack_dir_input.x -= 1;
//         if (is_key_down(SCANCODE_DOWN )) player_data.attack_dir_input.y -= 1;
//         if (is_key_down(SCANCODE_RIGHT)) player_data.attack_dir_input.x += 1;
//     }
//
//     if (player_data.attack_dir_input.x + player_data.attack_dir_input.y != 0) {
//         player_data.attack_input = 1;
//     }
//     else if (is_key_down(SCANCODE_Z) || is_key_down(SCANCODE_J)) {
//         player_data.attack_input = 1;
//         player_data.attack_dir_input = player_data.facing_dir;
//     }
//
//     // Move Animation
//     if (player_data.facing_dir.y == 1)
//         player_data.drawer_srect_pos = (Vec2){0, 20};
//     else if (player_data.facing_dir.y == -1)
//         player_data.drawer_srect_pos = (Vec2){0,  0};
//     else if (player_data.facing_dir.x == 1)
//         player_data.drawer_srect_pos = (Vec2){0, 60};
//     else
//         player_data.drawer_srect_pos = (Vec2){0, 40};
//
//     if (move_input_len > EPSILON) {
//         float delta = (float)cur_frame_time - player_data.last_move_frame_time;
//         if (delta > player_def.move_ani_delta) {
//             int n = (int)(delta / player_def.move_ani_delta);
//             const int MAX_FRAME = 4;
//             player_data.cur_move_frame = (player_data.cur_move_frame + n) % MAX_FRAME;
//
//             player_data.last_move_frame_time = (float)cur_frame_time + n * delta;
//         }
//
//         player_data.drawer_srect_pos.x = 0 + player_data.cur_move_frame * 20;
//     } else {
//         player_data.cur_move_frame = 0;
//         player_data.last_move_frame_time = cur_frame_time;
//     }
//
//     // Attack Animation
//     if (player_data.attacking) {
//         if (player_data.attack_dir.y == 1)
//             player_data.drawer_srect_pos = (Vec2){80, 20};
//         else if (player_data.attack_dir.y == -1)
//             player_data.drawer_srect_pos = (Vec2){80,  0};
//         else if (player_data.attack_dir.x == 1)
//             player_data.drawer_srect_pos = (Vec2){80, 60};
//         else
//             player_data.drawer_srect_pos = (Vec2){80, 40};
//
//         const int MAX_FRAME = 4;
//         if (player_data.cur_attack_frame < MAX_FRAME) {
//             float delta = (float)cur_frame_time - player_data.last_attack_frame_time;
//             // FIXME logic error btw
//             float cd = player_data.cur_attack_frame < 1 ? player_def.attack_hold_ani_delta
//                                                         : player_def.attack_act_ani_delta;
//             if (delta > cd) {
//                 int n = (int)(delta / cd);
//                 player_data.cur_attack_frame += n;
//                 if (player_data.cur_attack_frame > MAX_FRAME)
//                     player_data.cur_attack_frame = MAX_FRAME;
//
//                 player_data.last_attack_frame_time = (float)cur_frame_time + n * delta;
//             }
//         }
//
//         player_data.drawer_srect_pos.x = 80 + player_data.cur_attack_frame * 20;
//     } else {
//         player_data.cur_attack_frame = 0;
//         player_data.last_attack_frame_time = cur_frame_time;
//     }
// }
//
// void player_render_update() {
//     // img_get_drawer_ptr(&image_sys, player_data.drawer)->srect =
//     //     (Rect){player_data.drawer_srect_pos.x, player_data.drawer_srect_pos.y,
//     //            20, 20};
//     // img_feed_drawer_world(&image_sys, player_data.drawer, player_data.pos, 0, 1);
// }
//
// void player_draw() {
//     // img_draw(&image_sys, player_data.drawer);
// }
//
// void player_destroy() {}
