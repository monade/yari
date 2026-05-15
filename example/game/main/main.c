#define RAYCAST_MAIN
#include <raycast.h>
#include <stdio.h>
#include <hud.h>
#include "assets.h"
#include "fonts.h"

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


#define PLAYER_ROTATION_SPEED 1.25
#define PLAYER_SPEED 2.5



// 0 null, 1-127 texture_id, 128-255 color_id
static uint8_t map[ROWS][COLS] = {0};

void test_sprite_script(GameState *state, Sprite *self) {
    (void)state;
    float incX = sinf(get_time());
    float incY = cosf(get_time());
    self->pos.x += incX * 0.01;
    self->pos.y += incY * 0.01;
    if(fabs(incX) > 0.75 || fabs(incY) > 0.75) self->texture_id = tx_greenlight2;
    else self->texture_id = tx_greenlight;
}

typedef struct {
    int score;
} ExampleGameAttributes;

static ExampleGameAttributes attrs = {0};

Sprite sprites[] = {
    {.pos = {6.5, 4.5}, .texture_id = tx_pillar, .vmove=-0.7, .collision_threshold = 0.25, .collision_mask = 1},
    {.pos = {5.5, 5.5}, .texture_id = tx_greenlight, .update = test_sprite_script, .collision_threshold = 0.25, .collision_mask = 1},
    {.pos = {2.5, 3.5}, .texture_id = tx_barrel, .collision_threshold = 0.25, .collision_mask = 1},
    {.pos = {3.5, 3.5}, .texture_id = tx_barrel, .vmove=0.25, .collision_threshold = 0.25, .collision_mask = 1},
};
#define NUM_SPRITES ARRAY_LEN(sprites)



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
  state->sprites = sprites;
  state->num_sprites = NUM_SPRITES;
  state->player = (Player){.pos = {10.5, 5.5}, .dir = {0, 1}, .collision_threshold = 0.15};
  state->assets_map = assets_map;
  state->floor_texture = tx_greystone;
  state->ceil_texture = tx_greystone;
  state->state_attributes = &attrs;
}



void move_player(GameState *state) {
    Player *p = &state->player;
    if (is_key_down(KEY_A)) {
        p->dir = rotate(p->dir, COUNTERCLOCKWISE, PLAYER_ROTATION_SPEED);
    }
    if (is_key_down(KEY_D)) {
        p->dir = rotate(p->dir, CLOCKWISE, PLAYER_ROTATION_SPEED);
    }
    if (is_key_down(KEY_W)) {
        Vector2 next_pos = move(p->pos, p->dir, FORWARD, PLAYER_SPEED);
        if (!check_player_collision(state, next_pos)) {
          p->pos = next_pos;
        }
    }
    if (is_key_down(KEY_S)) {
        Vector2 next_pos = move(p->pos, p->dir, BACK, PLAYER_SPEED);
        if (!check_player_collision(state, next_pos)) {
          p->pos = next_pos;
        }
    }
    if (is_key_down(KEY_E)) {
        Vector2 next_pos = move(p->pos, p->dir, RIGHT, PLAYER_SPEED);
        if (!check_player_collision(state, next_pos)) {
            p->pos = next_pos;
        }
    }
    if (is_key_down(KEY_Q)) {
        Vector2 next_pos = move(p->pos, p->dir, LEFT, PLAYER_SPEED);
        if (!check_player_collision(state, next_pos)) {
            p->pos = next_pos;
        }
    }
}

void print_fps() {
    char buffer[48];
    snprintf(buffer, sizeof(buffer), "FPS:%02d dt:%dms",
             (int)get_fps(), (int)(get_frame_time() * 1000));
    draw_text(buffer, SCREEN_W - 130, SCREEN_H - 15, *fonts[FONT_SM], C_RED);
}

void draw_hud(const GameState* state) {
    const ExampleGameAttributes* a = state->state_attributes;
    char buffer[12];
    // SCORE
    draw_text("SCORE", 5, 17, *fonts[FONT_SM], C_WHITE);
    snprintf(buffer, sizeof(buffer), "%05d", a->score);
    draw_text(buffer, 5, 27, *fonts[FONT_SM], C_WHITE);
    // TIME
    const int time = (int)state->game_time / 1000;
    draw_text("TIME", SCREEN_W - 45, 17, *fonts[FONT_SM], C_WHITE);
    snprintf(buffer, sizeof(buffer), "%04d", time);
    draw_text(buffer, SCREEN_W - 45, 27, *fonts[FONT_SM], C_WHITE);
    // SPRITE
    for (int i = 0; i < 3; i++) {
        draw_asset((pixel_t*)assets_map[tx_barrel], 70 + i * 35, -17, 45, 45, 64);
    }
}

void update_state(GameState* state) {
    ExampleGameAttributes* a = state->state_attributes;
    a->score = (int)(state->game_time / 100);
}

void update_game(GameState *state) {
    draw_game();
    move_player(state);
    draw_hud(state);
    update_state(state);
#ifdef DEBUG
    print_fps();
#endif
}
