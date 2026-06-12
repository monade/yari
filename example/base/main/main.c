#define YARI_MAIN
#define YARI_NO_PREFIX
#include <yari.h>

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
        map[i][0] = 131;
        map[i][COLS - 1] = 131;
    }
    for (int j = 0; j < COLS; j++) {
        map[0][j] = 131;
        map[ROWS - 1][j] = 131;
    }

    // inner blocks
    map[7][7] = 130;
    map[8][8] = 129;
    map[9][9] = 134;
}

void move_player(GameState *state) {
    Player *p = &state->player;
    if (is_key_down(YR_KEY_A)) {
        p->dir = rotate(p->dir, YR_COUNTERCLOCKWISE, PLAYER_ROTATION_SPEED);
    }
    if (is_key_down(YR_KEY_D)) {
        p->dir = rotate(p->dir, YR_CLOCKWISE, PLAYER_ROTATION_SPEED);
    }

    Vector2 target = p->pos;
    if (is_key_down(YR_KEY_W)) target = move(target, p->dir, YR_FORWARD, PLAYER_SPEED);
    if (is_key_down(YR_KEY_S)) target = move(target, p->dir, YR_BACK, PLAYER_SPEED);
    if (is_key_down(YR_KEY_E)) target = move(target, p->dir, YR_RIGHT, PLAYER_SPEED);
    if (is_key_down(YR_KEY_Q)) target = move(target, p->dir, YR_LEFT, PLAYER_SPEED);

    p->pos = slide_collision(state, p->pos, target, NULL, p->collision_threshold, YR_CMSK_ALL);
}


// Main game functions
void yr_init_game(GameState *state) {
  init_map();
  state->screen_width = SCREEN_W;
  state->screen_height = SCREEN_H;
  state->map = (uint8_t *)map;
  state->map_cols = COLS;
  state->map_rows = ROWS;
  state->player = (Player){.pos = {14.5, 5.5}, .dir = {-0.8, 0.5}, .collision_threshold = 0.15};
}

void yr_update_game(GameState *state) {
    draw_game();
    move_player(state);
}
