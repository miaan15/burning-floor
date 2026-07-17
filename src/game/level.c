// #include "level.h"
// #include "global.h"
//
// void level_init(LevelData *lv, u64 seed) {
//     // random_reset(&lv->random, seed);
//     //
//     // int endgate_cnt = 1;
//     // double chance = .1;
//     // while (random_f(&lv->random) <= chance) {
//     //     ++endgate_cnt;
//     //     chance += .3;
//     //     if (chance > .6) chance = .6;
//     // }
//     //
//     // lv->endgates_len = endgate_cnt;
//     // for (size_t i = 0; i < lv->endgates_len; ++i) {
//     //     b2Vec2 pos = {0};
//     //     b2AABB aabb = {pos.x - .5, pos.y - .5, 1, 1};
//     //     lv->endgates[i] = (EndGate){
//     //         .ett = ett_new(&ett_mng, pos, aabb),
//     //         .drawer = img_make_drawer(&image_sys, 1)
//     //     };
//     // }
// }
//
// void level_destroy(LevelData *data) {
//     arena_destroy(&data->arena);
// }
