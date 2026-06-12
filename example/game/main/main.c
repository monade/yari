#define RAYCAST_MAIN
#include <raycast.h>
#include <stdio.h>
#include "assets.h"
#include "fonts.h"
#include "level.h"

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


JoystickConfig axes[2];

void update_mob(GameState *state, Entity *self, size_t index) {
    (void)state;
    (void)index;
    self->pos.x += 0.01f;
}

typedef struct {
    int hp;
    int gun;
} PlayerState;
PlayerState playerState;

void pickup_gun(GameState *state, Entity *self, size_t index) {

}

void move_player(GameState *state) {
    Player *p = &state->player;
    if (is_key_down(YARI_KEY_A)) {
        p->dir = rotate(p->dir, COUNTERCLOCKWISE, PLAYER_ROTATION_SPEED);
    }
    if (is_key_down(YARI_KEY_D)) {
        p->dir = rotate(p->dir, CLOCKWISE, PLAYER_ROTATION_SPEED);
    }

    Vector2 target = p->pos;
    if (is_key_down(YARI_KEY_W)) target = move(target, p->dir, FORWARD, PLAYER_SPEED);
    if (is_key_down(YARI_KEY_S)) target = move(target, p->dir, BACK, PLAYER_SPEED);
    if (is_key_down(YARI_KEY_E)) target = move(target, p->dir, RIGHT, PLAYER_SPEED);
    if (is_key_down(YARI_KEY_Q)) target = move(target, p->dir, LEFT, PLAYER_SPEED);

    CollisionInfo hit;
    p->pos = slide_collision(state, p->pos, target, &hit, p->collision_threshold, CMSK_PLAYER);
}

void init_game(GameState *state) {
  state->screen_width = SCREEN_W;
  state->screen_height = SCREEN_H;
  state->ray_res = RAY_RES;
  state->target_fps = TARGET_FPS;
  state->map = level_get_map();
  state->map_cols = MAP_COLS;
  state->map_rows = MAP_ROWS;
  state->assets_map = assets_map;
  state->floor_texture = LEVEL_FLOOR;
  state->ceil_texture = LEVEL_CEIL;
  state->player = init_player();

  level_append_exported_entities(&state->entities);

  joystick_init(32, 36, axes);

  playerState.hp = 100;
  playerState.gun = 0;
}

int game_state = 0;
void update_game(GameState *state) {
    if(game_state == 0) {
        draw_text("Press any key to start", 120, 30, fonts[FONT_MD], 0xFFFFFF);
        if(is_key_pressed(YARI_KEY_W)) {
            game_state = 1;
        }
    } else {
        draw_game();
        move_player(state);
    }
}
