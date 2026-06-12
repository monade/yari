#define RAYCAST_MAIN
#include <raycast.h>
#include "assets.h"

#define ARRAY_LEN(array) (sizeof(array) / sizeof(array[0]))

#ifdef ESP32
#define TARGET_FPS 30
#define SCREEN_W 240
#define SCREEN_H 136
#define RAY_RES 2
#else
#define TARGET_FPS 60
#define SCREEN_W 800
#define SCREEN_H 600
#define RAY_RES 1
#endif

#define COLS 100
#define ROWS 100
#define CMSK_ENTITY 1

#define PLAYER_ROTATION_SPEED 1.25
#define PLAYER_SPEED 2.5

#define CMSK_PROJECTILE 4
#define CMSK_OTHER 8
#define CMSK_PLAYER (CMSK_ALL & ~CMSK_PROJECTILE & ~CMSK_OTHER)

// 0 null, 1-127 texture_id, 128-255 color_id
static uint8_t map[ROWS][COLS] = {0};

void test_sprite_script(GameState *state, Entity *self, size_t index) {
    (void)state;
    (void)index;
    float incX = sinf(get_time());
    float incY = cosf(get_time());
    self->pos.x += incX * 0.01;
    self->pos.y += incY * 0.01;
    if(fabs(incX) > 0.75 || fabs(incY) > 0.75) self->texture_id = tx_greenlight2;
    else self->texture_id = tx_greenlight;
}

void test_pop_on_touch_script(GameState *state, Entity *self, size_t index) {
    if(self->dist < state->player.collision_threshold + self->collision_threshold) {
        da_remove_unordered(&state->entities, index);
    }
}

void test_projectile_script(GameState *state, Entity *self, size_t index) {
    self->pos = move(self->pos, state->player.dir, FORWARD, PLAYER_SPEED * 2);
    if(self->dist > MAX_RENDER_DIST) {
        da_remove_unordered(&state->entities, index);
        return;
    }
    CollisionInfo hit = check_collision(state, self->pos, self->collision_threshold, CMSK_ALL & ~CMSK_PROJECTILE & ~CMSK_OTHER);
    if (hit.type != COLLISION_NONE) {
        da_remove_unordered(&state->entities, index);
        if (hit.type == COLLISION_ENTITY) {
            hit.entity->disabled = true;
        } else if (hit.type == COLLISION_WALL) {
            int cell_x = hit.cell_x;
            int cell_y = hit.cell_y;
            if (cell_x >= 0 && cell_x < state->map_cols && cell_y >= 0 && cell_y < state->map_rows && state->map[cell_y * state->map_cols + cell_x] > 128) {
                state->map[cell_y * state->map_cols + cell_x] = 0;
            }
        }
    }
}

void spawn_test_entity(GameState *state) {
    da_append(&state->entities, ((Entity){.pos = {state->player.pos.x + state->player.dir.x, state->player.pos.y + state->player.dir.y}, .texture_id = tx_barrel, .update = test_projectile_script, .collision_threshold = 0.25, .collision_mask = CMSK_PROJECTILE, .hdiv= 0.7, .vdiv = 0.7}));
}

void init_map() {
    for (int i = 0; i < ROWS; i++) {
        map[i][0] = tx_bricks;
        map[i][COLS - 1] = tx_bricks;
    }
    for (int j = 0; j < COLS; j++) {
        map[0][j] = tx_bricks;
        map[ROWS - 1][j] = tx_bricks;
    }
    map[1][3] = tx_bricks;
    map[1][4] = 131;
    map[1][5] = 129;
    map[2][5] = 133;
    map[3][4] = 129;
    map[3][5] = tx_bricks;

    map[7][7] = 130;
    map[8][8] = 129;
    map[9][9] = 134;
}

void init_game(GameState *state) {
  init_map();
  state->screen_width = SCREEN_W;
  state->screen_height = SCREEN_H;
  state->ray_res = RAY_RES;
  state->target_fps = TARGET_FPS;
  state->map = (uint8_t *)map;
  state->map_cols = COLS;
  state->map_rows = ROWS;
  da_append(&state->entities, ((Entity){.pos = {6.5, 4.5}, .texture_id = tx_pillar, .vmove=-0.7, .collision_threshold = 0.25, .collision_mask = CMSK_ENTITY}));
  da_append(&state->entities, ((Entity){.pos = {5.5, 5.5}, .texture_id = tx_greenlight, .update = test_sprite_script, .collision_threshold = 0.25, .collision_mask = CMSK_ENTITY}));
  da_append(&state->entities, ((Entity){.pos = {2.5, 3.5}, .texture_id = tx_barrel, .update = test_pop_on_touch_script, .collision_threshold = 0.25, .collision_mask = CMSK_OTHER}));
  da_append(&state->entities, ((Entity){.pos = {3.5, 3.5}, .texture_id = tx_barrel, .vmove=0.25, .collision_threshold = 0.25, .collision_mask = CMSK_ENTITY}));
  state->player = (Player){.pos = {10.5, 5.5}, .dir = {0, 1}, .collision_threshold = 0.15};
  state->assets_map = assets_map;
  state->floor_texture = tx_greystone;
  state->ceil_texture = tx_greystone;
}


void move_player(GameState *state) {
    Player *p = &state->player;
    if (is_key_down(KEY_A)) {
        p->dir = rotate(p->dir, COUNTERCLOCKWISE, PLAYER_ROTATION_SPEED);
    }
    if (is_key_down(KEY_D)) {
        p->dir = rotate(p->dir, CLOCKWISE, PLAYER_ROTATION_SPEED);
    }

    Vector2 target = p->pos;
    if (is_key_down(KEY_W)) target = move(target, p->dir, FORWARD, PLAYER_SPEED);
    if (is_key_down(KEY_S)) target = move(target, p->dir, BACK, PLAYER_SPEED);
    if (is_key_down(KEY_E)) target = move(target, p->dir, RIGHT, PLAYER_SPEED);
    if (is_key_down(KEY_Q)) target = move(target, p->dir, LEFT, PLAYER_SPEED);

    CollisionInfo hit;
    p->pos = slide_collision(state, p->pos, target, &hit, p->collision_threshold, CMSK_PLAYER);
}

#ifdef DEBUG
#include <stdio.h>
void print_fps() {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "FPS: %.2f", get_fps());
    draw_text(buffer, 10, 10, 20, C_WHITE);
}
#endif

void update_game(GameState *state) {
    draw_game();
    move_player(state);
    if (is_key_down(KEY_SPACE)) {
        spawn_test_entity(state);
    }
#ifdef DEBUG
    print_fps();
#endif
}
