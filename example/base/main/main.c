#define RAYCAST_MAIN
#include <raycast.h>

#define SCREEN_W 800
#define SCREEN_H 600

#define COLS 20
#define ROWS 20

#define PLAYER_ROTATION_SPEED 1.25
#define PLAYER_SPEED 2.5

// 0 null, 1-127 texture_id, 128-255 color_id
static uint8_t map[ROWS][COLS] = {0};


void init_map() {
    // border
    for (int i = 0; i < ROWS; i++) {
        map[i][0] = 128;
        map[i][COLS - 1] = 128;
    }
    for (int j = 0; j < COLS; j++) {
        map[0][j] = 128;
        map[ROWS - 1][j] = 128;
    }

    // inner blocks
    map[7][7] = 130;
    map[8][8] = 129;
    map[9][9] = 134;
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

    p->pos = slide_collision(state, p->pos, target, NULL, p->collision_threshold, CMSK_ALL);
}

void init_game(GameState *state) {
  init_map();
  state->screen_width = SCREEN_W;
  state->screen_height = SCREEN_H;
  state->map = (uint8_t *)map;
  state->map_cols = COLS;
  state->map_rows = ROWS;
  state->player = (Player){.pos = {14.5, 5.5}, .dir = {-0.8, 0.5}, .collision_threshold = 0.15};
}

void update_game(GameState *state) {
    draw_game();
    move_player(state);
}
