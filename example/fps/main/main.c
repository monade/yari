#define YARI_MAIN
#define YARI_NO_PREFIX
#include <yari.h>
#include <yari_utils.h>
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
#define SCREEN_W 1200
#define SCREEN_H 600
#endif

#define PLAYER_ROTATION_SPEED 2.0
#define PLAYER_SPEED 5.0
#define PLAYER_COLLISION_THRESHOLD 0.15f

#define GUN_SCALE 0.45f
#define HAND_COOLDOWN 0.7f
#define GUN_COOLDOWN 0.4f
#define SHOTGUN_COOLDOWN 0.8f

#define ANIMATION_SPEED 0.25f

#define MUMMY_SPEED 4.0
#define MUMMY_DAMAGE_COOLDOWN 0.5
#define MUMMY_SPAWN_INTERVAL 2.5
#define PROJECTILE_SPEED 8.0f

#define SPAWN_POINT_COOLDOWN 24.8f

int joystick_id;
typedef struct {
    int hp;
    int gun;
    bool has_key;
    bool has_killed_boss;
    Timer damage_cd;
    Timer shot_cd;
    Timer shot_animation;
    bool is_taking_damage;
} PlayerData;

typedef struct {
    int state;
    PlayerData player;
} GameData;

typedef struct {
    int hp;
    Timer hit_animation;
    Timer shot_cd;
    bool is_boss;
} EnemyData;

typedef struct {
    Vector2 dir;
} ProjectileData;

GameData game = {0};

Timer second_wave_cd = {0};
Timer third_wave_cd = {0};
Timer fourth_wave_cd = {0};
Timer boss_animation = {0};

struct v2i {
    int x;
    int y;
};

struct v2i second_wave_trigger_points[4] = {
        {5,11},
        {6,11},
        {7,11},
        {8,11}
    };

struct v2i third_wave_trigger_points[5] = {
    {14,7},
    {15, 7},
    {16,7},
    {17,7},
    {18,7}
};

struct v2i fourth_wave_trigger_points[5] = {
    {39, 32},
    {40, 32},
    {41, 32},
    {42, 32},
    {42, 33}
};

struct v2i boss_trigger = {.x = 48, .y = 17};

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
        if(game.player.has_key && game.player.has_killed_boss) {
            game.state = 2;
        } else if (!game.player.has_key) {
            draw_text("You need the key to exit!", state->screen_width / 2 - 120, state->screen_height / 2 - 10, fonts[YR_FONT_MD], YR_RED);
        } else if (!game.player.has_killed_boss) {
            draw_text("Kill the boss!", state->screen_width / 2 - 120, state->screen_height / 2 - 10, fonts[YR_FONT_MD], YR_RED);
        }
    }
}

void update_mummy(YrGameState *state, YrEntity *self, size_t index) {
    (void)index;
    EnemyData *data = (EnemyData*) self->entity_data;
    if (!timer_is_done(&data->hit_animation)) {
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

    if (self->dist > 0 && contact > self->dist) {
        game.player.is_taking_damage = true;
        if (timer_loop(&game.player.damage_cd, MUMMY_DAMAGE_COOLDOWN)) {
            game.player.hp -= 5;
        }
        return;
    }

    Vector2 dir = Vector2Subtract(state->camera.pos, self->pos);
    dir = Vector2Normalize(dir);
    Vector2 target = Vector2Add(self->pos, Vector2Scale(dir, MUMMY_SPEED * get_frame_time()));
    self->pos = slide_collision_with_radius(state, self->pos, target, NULL, self->collision_threshold, YR_CMSK_WALL | YR_CMSK_ENEMY, .5f);
}

void update_boss(YrGameState *state, YrEntity *self, size_t index) {
    (void)index;
    float x_slide = sinf(get_time() * 2.0f) * 0.5f;
    self->pos.x += x_slide * get_frame_time();

    EnemyData *data = (EnemyData*) self->entity_data;
    if (!timer_is_done(&data->hit_animation)) {
        self->texture_id = tx_spr_051;
    } else {
        int stime = (int)(get_time()*3);
        if(stime % 2) {
            self->texture_id = tx_spr_049;
        } else {
            self->texture_id = tx_spr_052;
        }
    }

    if ( timer_loop(&data->shot_cd, 1.5f)) {
        Vector2 dir = Vector2Subtract(state->camera.pos, self->pos);
        dir = Vector2Normalize(dir);
        Vector2 projectile_pos = Vector2Add(self->pos, Vector2Scale(dir, 0.1f));
        ProjectileData *p = malloc(sizeof(*p));
        p->dir = dir;
        Entity projectile = create_boss_projectile_pos(projectile_pos, p);
        da_append(&state->entities, projectile);
    }
}

void update_boss_projectile(YrGameState *state, YrEntity *self, size_t index) {
    (void)index;
    
    ProjectileData *p = (ProjectileData*) self->entity_data;
    if (self->dist > 0 && self->dist < PLAYER_COLLISION_THRESHOLD + self->collision_threshold) {
        game.player.hp -= 10;
        game.player.is_taking_damage = true;
        da_remove_unordered(&state->entities, index);
        free(p);
        return;
    }
    Vector2 target = Vector2Add(self->pos, Vector2Scale(p->dir, PROJECTILE_SPEED * get_frame_time()));
    CollisionInfo hit = check_collision(state, target, self->collision_threshold, YR_CMSK_WALL);
    if (hit.type == YR_COLLISION_WALL) {
        da_remove_unordered(&state->entities, index);
        free(p);
        return;
    }
    self->pos = target;
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
    bool shot_animation_done = timer_is_done(&game.player.shot_animation);
    switch(game.player.gun) {
        case 0: {
            if (!shot_animation_done) {
                gun_asset_id = tx_wep_hnd1;
            } else {
                gun_asset_id = tx_wep_hnd0;
            }
        } break;
        case 1: {
            if (!shot_animation_done) {
                gun_asset_id = tx_wep_gun1;
            } else {
                gun_asset_id = tx_wep_gun0;
            }
        } break;
        case 2: {
            if (!shot_animation_done) {
                gun_asset_id = tx_wep_bfg1;
            } else {
                gun_asset_id = tx_wep_bfg0;
            }
        } break;
    }

    if(gun_asset_id) {
        const int gun_size = SCREEN_H * GUN_SCALE;
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
    EnemyData *data = malloc(sizeof(*data));
    data->hp = 100;
    data->is_boss = false;
    Entity e = create_mummy_pos(pos, data);
    da_append(&state->entities, e);
}

void spawn_boss(GameState *state) {
    EnemyData *data = malloc(sizeof(*data));
    data->hp = 500;
    data->is_boss = true;
    Entity e = create_boss(data);
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

void spawn_fourth_wave(GameState *state) {
    spawn_mummy(state, (Vector2){ .x = 46, .y = 23 });
    spawn_mummy(state, (Vector2){ .x = 46, .y = 25 });
    spawn_mummy(state, (Vector2){ .x = 47, .y = 23 });
    spawn_mummy(state, (Vector2){ .x = 43, .y = 26 });
    spawn_mummy(state, (Vector2){ .x = 46, .y = 28 });
}

void check_monster_spawns(GameState *state) {
    Vector2 player_pos = state->camera.pos;

    for(size_t i = 0; i < ARRAY_LEN(second_wave_trigger_points); i++) {
        if ((int)player_pos.x == second_wave_trigger_points[i].x
            && (int)player_pos.y == second_wave_trigger_points[i].y
            && timer_loop(&second_wave_cd, SPAWN_POINT_COOLDOWN)) {
            spawn_second_wave(state);
            break;
        }
    }

    for(size_t i = 0; i < ARRAY_LEN(third_wave_trigger_points); i++) {
        if ((int)player_pos.x == third_wave_trigger_points[i].x
            && (int)player_pos.y == third_wave_trigger_points[i].y
            && timer_loop(&third_wave_cd, SPAWN_POINT_COOLDOWN)) {
            spawn_third_wave(state);
            break;
        }
    }

    for(size_t i = 0; i < ARRAY_LEN(fourth_wave_trigger_points); i++) {
        if ((int)player_pos.x == fourth_wave_trigger_points[i].x
            && (int)player_pos.y == fourth_wave_trigger_points[i].y
            && timer_loop(&fourth_wave_cd, SPAWN_POINT_COOLDOWN)) {
            spawn_fourth_wave(state);
            break;
        }
    }

    if ((int)player_pos.x == boss_trigger.x
        && (int)player_pos.y == boss_trigger.y && !timer_is_started(&boss_animation)) {
        spawn_boss(state);
        boss_animation = timer_start(2.0f);
    }
    if(timer_is_started(&boss_animation)) {
        if (!timer_is_done(&boss_animation)) {
            state->camera.horizon = sinf(get_time() * 100.0f) * 0.05f;
            state->camera.angle = sinf(get_time() * 50.0f) * 0.05f;
        } else {
            state->camera.horizon = 0.0f;
            state->camera.angle = 0.0f;
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

void shoot_gun(GameState *state) {
    if(!timer_is_done(&game.player.shot_cd)) return;

    
    float shot_cd, range; int damage;
    switch(game.player.gun) {
    case 0: damage =  10; range = 1.5; shot_cd = HAND_COOLDOWN;    break;
    case 1: damage =  35; range =  30; shot_cd = GUN_COOLDOWN;     break;
    case 2: damage = 100; range =   5; shot_cd = SHOTGUN_COOLDOWN; break;
    }
    game.player.shot_cd = timer_start(shot_cd);
    game.player.shot_animation = timer_start(ANIMATION_SPEED);

    CollisionInfo hit = check_ray_collision(state, state->camera.pos, state->camera.dir, range, YR_CMSK_ENEMY);
    if (hit.entity == NULL) return;

    EnemyData *data = (EnemyData*) hit.entity->entity_data;
    data->hp -= damage;
    data->hit_animation = timer_start(ANIMATION_SPEED);


    if (data->hp <= 0) {
        if (data->is_boss) {
            game.player.has_killed_boss = true;
        }
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

        if (is_key_down(YR_KEY_SPACE)) {
            shoot_gun(state);
        }

        if (game.player.hp <= 0) {
            game.state = 3;
        }

        if (game.player.is_taking_damage) {
            draw_rectangle(0, state->screen_height - 8, state->screen_width, 8, YR_RED);
            game.player.is_taking_damage = false;
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
