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

#define PLAYER_ROTATION_SPEED 2.0
#define PLAYER_SPEED 5.0
#define PLAYER_COLLISION_THRESHOLD 0.15f

#define GUN_SCALE 0.35f
#define GUN_COOLDOWN 0.4f
#define SHOTGUN_COOLDOWN 0.8f

#define ANIMATION_SPEED 0.25f

#define MUMMY_SPEED 4.0
#define MUMMY_DAMAGE_COOLDOWN 0.5
#define MUMMY_SPAWN_INTERVAL 2.5

#define SPAWN_POINT_COOLDOWN 24.8f

int joystick_id;
typedef struct {
    int hp;
    int gun;
    bool has_key;
    float last_hit_time;
    float last_shot_time;
} PlayerData;

typedef struct {
    int state;
    float last_spawn_time;
    PlayerData player;
} GameData;

typedef struct {
    int hp;
    float last_hit_time;
} MummyData;

GameData game;

float second_wave_activated = 0.0f;
float third_wave_activated = 0.0f;

Vector2 second_wave_trigger_points[4] = {
        {5,11},
        {6,11},
        {7,11},
        {8,11}
    };

Vector2 third_wave_trigger_points[5] = {
    {14,7},
    {15, 7},
    {16,7},
    {17,7},
    {18,7}
};

void pickup_key(YrGameState *state, YrEntity *self, size_t index) {
    if (self->dist < PLAYER_COLLISION_THRESHOLD + self->collision_threshold) {
        game.player.has_key = true;
        da_remove_unordered(&state->entities, index);
    }
}

void trigger_end(YrGameState *state, YrEntity *self, size_t index) {
    (void)state;
    (void)index;
    if (self->dist < PLAYER_COLLISION_THRESHOLD + self->collision_threshold) {
        if(game.player.has_key) {
            game.state = 2;
        }
    }
}

void update_mummy(YrGameState *state, YrEntity *self, size_t index) {
    (void)index;
    MummyData *data = (MummyData*) self->entity_data;
    if (data->last_hit_time + ANIMATION_SPEED >= get_time()) {
        self->texture_id = tx_spr_008;
    } else {
        int stime = (int)(get_time()*3);
        if(stime % 2) {
            self->texture_id = tx_spr_010;
        } else {
            self->texture_id = tx_spr_009;
        }
    }

    float contact = PLAYER_COLLISION_THRESHOLD + self->collision_threshold * 2;

    if (contact > self->dist) {
        if (get_time() - game.player.last_hit_time >= MUMMY_DAMAGE_COOLDOWN) {
            game.player.hp -= 5;
            game.player.last_hit_time = get_time();
        }
        return;
    }

    Vector2 dir = Vector2Subtract(state->camera.pos, self->pos);
    dir = Vector2Normalize(dir);
    Vector2 target = Vector2Add(self->pos, Vector2Scale(dir, MUMMY_SPEED * get_frame_time()));
    self->pos = slide_collision_with_radius(state, self->pos, target, NULL, self->collision_threshold, YR_CMSK_WALL | YR_CMSK_ENEMY, .5f);
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
    sprintf(hp_text, "HP: %d", game.player.hp);
    draw_text(hp_text, 10, 15, fonts[YR_FONT_SM], YR_GREEN);

    int gun_asset_id = 0;
    switch(game.player.gun) {
        case 0: gun_asset_id = tx_wep_hnd0; break;
        case 1: {
            if (get_time() - game.player.last_shot_time <= ANIMATION_SPEED) {
                gun_asset_id = tx_wep_gun1;
            } else {
                gun_asset_id = tx_wep_gun0;
            }
        } break;
        case 2: {
            if (get_time() - game.player.last_shot_time <= ANIMATION_SPEED) {
                gun_asset_id = tx_wep_bfg1;
            } else {
                gun_asset_id = tx_wep_bfg0;
            }
        } break;
    }

    if(gun_asset_id) {
        const int gun_size = SCREEN_W * GUN_SCALE;
        draw_texture((state->screen_width - gun_size) /2 , state->screen_height - gun_size, gun_size, gun_size, assets_map[gun_asset_id], 128, 128, true);
    }

    if(game.player.has_key) {
        draw_texture(0, state->screen_height - 55, 64, 64, assets_map[tx_spr_092], 64, 64, true);
    }
}

void print_fps() {
    float fps = get_fps();
    char fps_text[32];
    sprintf(fps_text, "FPS: %.1f", fps);
    draw_text(fps_text, SCREEN_W - 100, 15, fonts[YR_FONT_SM], YR_WHITE);
}

void spawn_mummy(GameState *state, Vector2 pos) {
    MummyData *data = malloc(sizeof(*data));
    data->hp = 100;
    Entity e = create_mummy_pos(pos, data);
    da_append(&state->entities, e);
}

void spawn_first_wave(GameState *state) {
    spawn_mummy(state, (Vector2){ .x = 11, .y = 25 });
    spawn_mummy(state, (Vector2){ .x = 12, .y = 29 });
    spawn_mummy(state, (Vector2){ .x = 9, .y = 26 });
    spawn_mummy(state, (Vector2){ .x = 8, .y = 35 });
    spawn_mummy(state, (Vector2){ .x = 8, .y = 29 });
}

void spawn_second_wave(GameState *state) {
    spawn_mummy(state, (Vector2){ .x = 11, .y = 3 });
    spawn_mummy(state, (Vector2){ .x = 12, .y = 2 });
    spawn_mummy(state, (Vector2){ .x = 9, .y = 3 });
    spawn_mummy(state, (Vector2){ .x = 4, .y = 5 });
    spawn_mummy(state, (Vector2){ .x = 2, .y = 2 });
}

void spawn_third_wave(GameState *state) {
    spawn_mummy(state, (Vector2){ .x = 13, .y = 15 });
    spawn_mummy(state, (Vector2){ .x = 21, .y = 13 });
    spawn_mummy(state, (Vector2){ .x = 23, .y = 13 });
    spawn_mummy(state, (Vector2){ .x = 17, .y = 16 });
    spawn_mummy(state, (Vector2){ .x = 27, .y = 5 });
    spawn_mummy(state, (Vector2){ .x = 16, .y = 23 });
}

void check_monster_spawns(GameState *state) {
    Vector2 player_pos = state->camera.pos;

    if (second_wave_activated == 0.0
        || (get_time() - second_wave_activated > SPAWN_POINT_COOLDOWN)
        ) {
        for(size_t i = 0; i < ARRAY_LEN(second_wave_trigger_points); i++) {
            if ((int)player_pos.x == (int)second_wave_trigger_points[i].x
                && (int)player_pos.y == (int)second_wave_trigger_points[i].y) {
                spawn_second_wave(state);
                second_wave_activated = get_time();
            }
        }
    }

    if (third_wave_activated == 0.0
        || (get_time() - third_wave_activated > SPAWN_POINT_COOLDOWN)) {
        for(size_t i = 0; i < ARRAY_LEN(third_wave_trigger_points); i++) {
            if ((int)player_pos.x == (int)third_wave_trigger_points[i].x
                && (int)player_pos.y == (int)third_wave_trigger_points[i].y) {
                spawn_third_wave(state);
                third_wave_activated = get_time();
            }
        }
    }

}

void pickup_gun(YrGameState *state, YrEntity *self, size_t index) {

    if(self->dist < PLAYER_COLLISION_THRESHOLD + self->collision_threshold) {
        game.player.gun = 1;
        da_remove_unordered(&state->entities, index);
        spawn_first_wave(state);
    }
}

void pickup_shotgun(YrGameState *state, YrEntity *self, size_t index) {

    if(self->dist < PLAYER_COLLISION_THRESHOLD + self->collision_threshold) {
        game.player.gun = 2;
        da_remove_unordered(&state->entities, index);
        spawn_first_wave(state);
    }
}

bool player_can_shoot() {
    float cd = game.player.gun == 1 ? GUN_COOLDOWN : SHOTGUN_COOLDOWN;
    return game.player.gun > 0 && get_time() - game.player.last_shot_time >= cd;
}

void shoot_gun(GameState *state) {
    if(!player_can_shoot()) return;

    game.player.last_shot_time = get_time();

    float range = game.player.gun == 1 ? 30 : 5;
    int damage = game.player.gun == 1 ? 35 : 100;

    CollisionInfo hit = check_ray_collision(state, state->camera.pos, state->camera.dir, range, YR_CMSK_ENEMY);
    if (hit.entity == NULL) return;

    MummyData *data = (MummyData*) hit.entity->entity_data;
    data->hp -= damage; // potrebbe varirare in base alla gun
    data->last_hit_time = get_time();


    if (data->hp <= 0) {
        da_remove_unordered(&state->entities, hit.entity_index);
        free(data);
    }

}

// Main game functions
void yr_init_game(GameState *state) {
  state->screen_width = SCREEN_W;
  state->screen_height = SCREEN_H;
  state->ray_res = RAY_RES;
  state->target_fps = TARGET_FPS;
  load_level(state);

  game.player.hp = 100;
  game.player.gun = 0;

  joystick_id = esp_joystick_init(32, 36);
  esp_key_init(25, YR_KEY_Q);
  esp_key_init(2, YR_KEY_E);
  esp_key_init(15, YR_KEY_X);
  esp_key_init(26, YR_KEY_SPACE);
}

void yr_update_game(GameState *state) {
    switch(game.state) {
    case 0:
        clear_screen(YR_BLACK);
        draw_text("Press SPACE to start", state->screen_width/2-140, state->screen_height/2-10, fonts[YR_FONT_MD], YR_WHITE);
        if(is_key_pressed(YR_KEY_SPACE)) {
            game.state = 1;
        }
        break;
    case 1:
        draw_game();
        draw_hud(state);
        move_player(state);

        check_monster_spawns(state);

        if (is_key_pressed(YR_KEY_SPACE)) {
            shoot_gun(state);
        }

        if (game.player.hp <= 0) {
            game.state = 3;
        }

        print_fps();
        break;
    case 2:
        clear_screen(YR_BLACK);
        draw_text("You win!", state->screen_width/2-60, state->screen_height/2-10, fonts[YR_FONT_MD], YR_WHITE);
        break;
    case 3:
        clear_screen(YR_BLACK);
        draw_text("Game Over", state->screen_width/2-60, state->screen_height/2-10, fonts[YR_FONT_MD], YR_WHITE);
        break;
    }
}
