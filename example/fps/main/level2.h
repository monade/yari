// File generated automatically by map_builder.c. DO NOT EDIT.
// MAP_BUILDER_STATE_BEGIN
// version 4
// size 50 50
// surface floor tx_brick
// surface ceil NULL_ASSET
// player_pos 23.6776161 27.2402802 0 1
// level_suffix 1
// entity entity_1 tx_spr_066 23.691864 30.4602013 0 0 0 0 0.400000006 0x00000004 1 - 0 - - dummy1
// entity entity_2 tx_spr_072 20.6575165 28.9576645 0 0 0 0 0.400000006 0x00000004 1 - 0 - - dummy2
// entity entity_3 tx_spr_050 27.967207 28.7741699 0 0 0 0 0.400000006 0x00000004 1 - 0 - - mummy2_attack
// entity entity_4 tx_spr_007 21.462471 24.551239 0 0 0 0 0.400000006 0x00000004 1 - 0 - - mummy_attack
// entity entity_5 tx_spr_024 27.536396 24.3400478 0 0 0 0 0.400000006 0x00000004 1 - 0 - - dummy3
// entity entity_6 tx_spr_030 24.3103409 23.0574265 0 0 0 0 0.400000006 0x00000004 1 - 0 - - dummy4
// MAP_BUILDER_STATE_END

#ifndef YR_LEVEL_H_LEVEL2
#define YR_LEVEL_H_LEVEL2

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <yari.h>
#include "assets.h"
#include "level_gen.h"

#define YR_MAP_COLS_LEVEL2 50
#define YR_MAP_ROWS_LEVEL2 50

#define YR_LEVEL2_FLOOR tx_brick
#define YR_LEVEL2_CEIL NULL_ASSET

static inline YrCamera init_camera_pos_level2(Vector2 pos) {
    return (YrCamera){
        .pos = pos,
        .dir = (Vector2){0.0f, 1.0f},
        .horizon = 0.0f,
    };
}

static inline YrCamera init_camera_level2(void) {
    return init_camera_pos_level2((Vector2){23.677616f, 27.24028f});
}

static const YrEntity entity_1_level2_template = {
    .pos = {23.691864f, 30.460201f},
    .texture_id = tx_spr_066,
    .vscale = 0.0f,
    .hscale = 0.0f,
    .vmove = 0.0f,
    .disabled = false,
    .kind = 0,
    .collision_mask = 0x00000004u,
    .collision_threshold = 0.4f,
};

static inline YrEntity create_entity_1_level2_pos(Vector2 pos, void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    YrEntity e = entity_1_level2_template;
    e.pos = pos;
    e.entity_data = data;
    e.init = init;
    e.update = update;
    e.cleanup = cleanup;
    yr_start_loop_animation(&e.animation, DUMMY1_ANIM);
    return e;
}

static inline YrEntity create_entity_1_level2(void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    return create_entity_1_level2_pos(entity_1_level2_template.pos, data, init, update, cleanup);
}

static const YrEntity entity_2_level2_template = {
    .pos = {20.657516f, 28.957664f},
    .texture_id = tx_spr_072,
    .vscale = 0.0f,
    .hscale = 0.0f,
    .vmove = 0.0f,
    .disabled = false,
    .kind = 0,
    .collision_mask = 0x00000004u,
    .collision_threshold = 0.4f,
};

static inline YrEntity create_entity_2_level2_pos(Vector2 pos, void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    YrEntity e = entity_2_level2_template;
    e.pos = pos;
    e.entity_data = data;
    e.init = init;
    e.update = update;
    e.cleanup = cleanup;
    yr_start_loop_animation(&e.animation, DUMMY2_ANIM);
    return e;
}

static inline YrEntity create_entity_2_level2(void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    return create_entity_2_level2_pos(entity_2_level2_template.pos, data, init, update, cleanup);
}

static const YrEntity entity_3_level2_template = {
    .pos = {27.967207f, 28.77417f},
    .texture_id = tx_spr_050,
    .vscale = 0.0f,
    .hscale = 0.0f,
    .vmove = 0.0f,
    .disabled = false,
    .kind = 0,
    .collision_mask = 0x00000004u,
    .collision_threshold = 0.4f,
};

static inline YrEntity create_entity_3_level2_pos(Vector2 pos, void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    YrEntity e = entity_3_level2_template;
    e.pos = pos;
    e.entity_data = data;
    e.init = init;
    e.update = update;
    e.cleanup = cleanup;
    yr_start_loop_animation(&e.animation, MUMMY2_ATTACK_ANIM);
    return e;
}

static inline YrEntity create_entity_3_level2(void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    return create_entity_3_level2_pos(entity_3_level2_template.pos, data, init, update, cleanup);
}

static const YrEntity entity_4_level2_template = {
    .pos = {21.462471f, 24.551239f},
    .texture_id = tx_spr_007,
    .vscale = 0.0f,
    .hscale = 0.0f,
    .vmove = 0.0f,
    .disabled = false,
    .kind = 0,
    .collision_mask = 0x00000004u,
    .collision_threshold = 0.4f,
};

static inline YrEntity create_entity_4_level2_pos(Vector2 pos, void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    YrEntity e = entity_4_level2_template;
    e.pos = pos;
    e.entity_data = data;
    e.init = init;
    e.update = update;
    e.cleanup = cleanup;
    yr_start_loop_animation(&e.animation, MUMMY_ATTACK_ANIM);
    return e;
}

static inline YrEntity create_entity_4_level2(void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    return create_entity_4_level2_pos(entity_4_level2_template.pos, data, init, update, cleanup);
}

static const YrEntity entity_5_level2_template = {
    .pos = {27.536396f, 24.340048f},
    .texture_id = tx_spr_024,
    .vscale = 0.0f,
    .hscale = 0.0f,
    .vmove = 0.0f,
    .disabled = false,
    .kind = 0,
    .collision_mask = 0x00000004u,
    .collision_threshold = 0.4f,
};

static inline YrEntity create_entity_5_level2_pos(Vector2 pos, void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    YrEntity e = entity_5_level2_template;
    e.pos = pos;
    e.entity_data = data;
    e.init = init;
    e.update = update;
    e.cleanup = cleanup;
    yr_start_loop_animation(&e.animation, DUMMY3_ANIM);
    return e;
}

static inline YrEntity create_entity_5_level2(void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    return create_entity_5_level2_pos(entity_5_level2_template.pos, data, init, update, cleanup);
}

static const YrEntity entity_6_level2_template = {
    .pos = {24.310341f, 23.057426f},
    .texture_id = tx_spr_030,
    .vscale = 0.0f,
    .hscale = 0.0f,
    .vmove = 0.0f,
    .disabled = false,
    .kind = 0,
    .collision_mask = 0x00000004u,
    .collision_threshold = 0.4f,
};

static inline YrEntity create_entity_6_level2_pos(Vector2 pos, void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    YrEntity e = entity_6_level2_template;
    e.pos = pos;
    e.entity_data = data;
    e.init = init;
    e.update = update;
    e.cleanup = cleanup;
    yr_start_loop_animation(&e.animation, DUMMY4_ANIM);
    return e;
}

static inline YrEntity create_entity_6_level2(void *data, YrEntityInitFunc init, YrEntityUpdateFunc update, YrEntityCleanupFunc cleanup) {
    return create_entity_6_level2_pos(entity_6_level2_template.pos, data, init, update, cleanup);
}

static const YrEntity *const level_exported_entities_level2[] = {
    &entity_1_level2_template,
    &entity_2_level2_template,
    &entity_3_level2_template,
    &entity_4_level2_template,
    &entity_5_level2_template,
    &entity_6_level2_template,
};

static const YrAnimation *const level_exported_animations_level2[] = {
    &DUMMY1_ANIM,
    &DUMMY2_ANIM,
    &MUMMY2_ATTACK_ANIM,
    &MUMMY_ATTACK_ANIM,
    &DUMMY3_ANIM,
    &DUMMY4_ANIM,
};

static inline void level_append_exported_entities_level2(YrContext *ctx) {
    for (size_t i = 0; i < sizeof(level_exported_entities_level2) / sizeof(level_exported_entities_level2[0]); i++) {
        YrEntity e = *level_exported_entities_level2[i];
        if (level_exported_animations_level2[i]) yr_start_loop_animation(&e.animation, *level_exported_animations_level2[i]);
        yr_create_entity(ctx, e);
    }
}

static const YrWall level_map_level2[YR_MAP_ROWS_LEVEL2 * YR_MAP_COLS_LEVEL2] = {
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
        YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(), YrEmptyWall(),
};

static inline void load_level2(YrContext *ctx) {
    ctx->assets_map = assets_map;
    ctx->map.walls = realloc(ctx->map.walls, sizeof(level_map_level2));
    memcpy(ctx->map.walls, level_map_level2, sizeof(level_map_level2));
    free(ctx->map.floor);
    ctx->map.floor = NULL;
    free(ctx->map.ceil);
    ctx->map.ceil = NULL;
    ctx->map.cols = YR_MAP_COLS_LEVEL2;
    ctx->map.rows = YR_MAP_ROWS_LEVEL2;
    ctx->map.floor_texture = YR_LEVEL2_FLOOR;
    ctx->map.ceil_texture = YR_LEVEL2_CEIL;
    ctx->camera = init_camera_level2();
    level_append_exported_entities_level2(ctx);
}

#endif // YR_LEVEL_H_LEVEL2
