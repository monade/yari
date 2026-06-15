#define YARI_MAIN
#define YARI_NO_PREFIX
#include <yari.h>
#include <stdio.h>
#include "assets.h"
#include "fonts.h"
#include "level.h"

#define RAY_RES 1
#ifdef ESP32
#define TARGET_FPS 30
#define SCREEN_W 240
#define SCREEN_H 136
#else
#define TARGET_FPS 60
#define SCREEN_W 800
#define SCREEN_H 600
#endif

#define PLAYER_ROTATION_SPEED 1.25
#define PLAYER_SPEED 3.0
#define PLAYER_COLLISION_THRESHOLD 0.15f

#define GUN_SCALE 0.35f


int joystick_id;

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
int game_state = 0;

void pickup_gun(GameState *state, Entity *self, size_t index) {
    float pickup_threshold = PLAYER_COLLISION_THRESHOLD + self->collision_threshold;
    if (self->dist < pickup_threshold) {
        playerState.gun = 1;
        da_remove_unordered(&state->entities, index);
    }
}


void move_player(GameState *state) {
    Camera *p = &state->camera;
    float joy_x = esp_joystick_get_axis(joystick_id, YR_X_AXIS);
    float joy_y = esp_joystick_get_axis(joystick_id, YR_Y_AXIS);

    if (is_key_down(YR_KEY_A) || joy_x < -0.5f) {
        p->dir = rotate(p->dir, YR_COUNTERCLOCKWISE, PLAYER_ROTATION_SPEED);
    }
    if (is_key_down(YR_KEY_D) || joy_x > 0.5f) {
        p->dir = rotate(p->dir, YR_CLOCKWISE, PLAYER_ROTATION_SPEED);
    }

    Vector2 target = p->pos;
    if (is_key_down(YR_KEY_W) || joy_y > 0.5f) target = move(target, p->dir, YR_FORWARD, PLAYER_SPEED);
    if (is_key_down(YR_KEY_S) || joy_y < -0.5f) target = move(target, p->dir, YR_BACK, PLAYER_SPEED);
    if (is_key_down(YR_KEY_E)) target = move(target, p->dir, YR_RIGHT, PLAYER_SPEED);
    if (is_key_down(YR_KEY_Q)) target = move(target, p->dir, YR_LEFT, PLAYER_SPEED);

    CollisionInfo hit;
    p->pos = slide_collision(state, p->pos, target, &hit, PLAYER_COLLISION_THRESHOLD, YR_CMSK_PLAYER);
}

void draw_hud(GameState *state) {
    (void)state;

    char hp_text[32];
    sprintf(hp_text, "HP: %d", playerState.hp);
    draw_text(hp_text, 10, 15, fonts[YR_FONT_SM], YR_GREEN);

    int gun_asset_id = 0;
    switch(playerState.gun) {
        case 0: gun_asset_id = tx_wep_hnd0; break;
        case 1: gun_asset_id = tx_wep_gun0; break;
    }

    if(gun_asset_id) {
        const int gun_size = SCREEN_W * GUN_SCALE;
        draw_texture((state->screen_width - gun_size) /2 , state->screen_height - gun_size, gun_size, gun_size, assets_map[gun_asset_id], 128, 128, true);
    }
}

void print_fps() {
    float fps = get_fps();
    char fps_text[32];
    sprintf(fps_text, "FPS: %.1f", fps);
    draw_text(fps_text, SCREEN_W - 100, 15, fonts[YR_FONT_SM], YR_WHITE);
}


// Main game functions
void yr_init_game(GameState *state) {
  state->screen_width = SCREEN_W;
  state->screen_height = SCREEN_H;
  state->ray_res = RAY_RES;
  state->target_fps = TARGET_FPS;
  load_level(state);

  playerState.hp = 100;
  playerState.gun = 0;

  joystick_id = esp_joystick_init(32, 36);
  esp_key_init(25, YR_KEY_Q);
  esp_key_init(2, YR_KEY_E);
  esp_key_init(15, YR_KEY_X);
  esp_key_init(26, YR_KEY_SPACE);
}

void yr_update_game(GameState *state) {
    if(game_state == 0) {
        clear_screen(YR_BLACK);
        draw_text("Press any key to start", 120, 30, fonts[YR_FONT_MD], YR_WHITE);
        if(is_key_pressed(YR_KEY_SPACE)) {
            game_state = 1;
        }
    } else {
        draw_game();
        draw_hud(state);
        move_player(state);

        print_fps();
    }
}
