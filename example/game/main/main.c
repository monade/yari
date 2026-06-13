#define RAYCAST_MAIN
#include <raycast.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

#define GUN_SCALE 0.35f

#define ENEMY_HP              3
#define ENEMY_SPEED           1.2f
#define ENEMY_DAMAGE          8
#define ENEMY_HIT_COOLDOWN_MS 700
#define GUN_RANGE             20.0f
#define GUN_AMMO              6
#define RELOAD_KEY            YARI_KEY_R
#define MAX_ENEMIES           8
#define SPAWN_INTERVAL_MS     2000
#define MIN_SPAWN_DIST        5.0f
#define MAX_SPAWN_DIST        14.0f
#define FIRE_KEY              YARI_KEY_SPACE

#define HIT_FLASH_MS          120
#define MUZZLE_FLASH_MS       60
#define MAX_CORPSES           5

#define ENEMY_TEX             tx_spr_078
#define ENEMY_HIT_TEX         tx_spr_008
#define CORPSE_TEX            tx_spr_011
#define CORPSE_VMOVE          0.12f  // small downward nudge so the corpse rests on the floor


JoystickConfig axes[2];

typedef struct {
    int hp;
    uint32_t hit_flash_until;
    bool dead;
    uint32_t death_time;
} EnemyData;

static uint32_t muzzle_flash_until = 0;

typedef struct {
    int hp;
    int gun;
    int ammo;
    int kills;
    uint32_t start_time;
    uint32_t survived_ms;
    uint32_t last_hit_time;
} PlayerState;
PlayerState playerState;

static uint32_t last_spawn_time = 0;

void pickup_gun(GameState *state, Entity *self, size_t index) {
    float pickup_threshold = state->player.collision_threshold + self->collision_threshold;
    if (self->dist < pickup_threshold) {
        playerState.gun = 1;
        playerState.ammo = GUN_AMMO;
        da_remove_unordered(&state->entities, index);
    }
}

// Corpses just sit there; kept renderable but inert (no collision, no AI).
void update_corpse(GameState *state, Entity *self, size_t index) {
    (void)state;
    (void)self;
    (void)index;
}

void update_enemy(GameState *state, Entity *self, size_t index) {
    Player *p = &state->player;
    float contact = p->collision_threshold + self->collision_threshold;

    EnemyData *ed = (EnemyData *)self->entity_data;
    self->texture_id = (state->game_time < ed->hit_flash_until) ? ENEMY_HIT_TEX : ENEMY_TEX;

    if (self->dist > contact) {
        Vector2 dir = Vector2Subtract(p->pos, self->pos);
        dir = Vector2Normalize(dir);
        float dt = get_frame_time();
        Vector2 target = Vector2Add(self->pos, Vector2Scale(dir, ENEMY_SPEED * dt));
        CollisionInfo hit;
        self->pos = slide_collision(state, self->pos, target, &hit, self->collision_threshold, CMSK_WALL);
    } else {
        if (state->game_time - playerState.last_hit_time >= ENEMY_HIT_COOLDOWN_MS) {
            playerState.hp -= ENEMY_DAMAGE;
            playerState.last_hit_time = state->game_time;
        }
    }
    (void)index;
}

void player_shoot(GameState *state) {
    if (playerState.gun == 0) return;
    if (is_key_pressed(RELOAD_KEY)) playerState.ammo = GUN_AMMO;
    if (!is_key_pressed(FIRE_KEY)) return;
    if (playerState.ammo <= 0) return;  // out of ammo: press R to reload

    playerState.ammo--;
    muzzle_flash_until = state->game_time + MUZZLE_FLASH_MS;

    CollisionInfo hit = check_ray_collision(state, state->player.pos, state->player.dir,
                                            GUN_RANGE, CMSK_ENTITY | CMSK_WALL);
    if (hit.type == COLLISION_ENTITY) {
        EnemyData *ed = (EnemyData *)hit.entity->entity_data;
        ed->hp--;
        if (ed->hp <= 0) {
            // Turn the enemy into an inert corpse instead of deleting it.
            ed->dead = true;
            ed->death_time = state->game_time;
            hit.entity->texture_id = CORPSE_TEX;
            hit.entity->vmove = CORPSE_VMOVE;
            hit.entity->collision_mask = CMSK_NONE;
            hit.entity->update = update_corpse;
            playerState.kills++;
        } else {
            ed->hit_flash_until = state->game_time + HIT_FLASH_MS;
        }
    }
}

// Drop the oldest corpses until at most MAX_CORPSES remain. Only ever touches
// corpses (update == update_corpse), so living enemies are never despawned.
void cull_corpses(GameState *state) {
    for (;;) {
        int corpses = 0;
        size_t oldest_idx = 0;
        uint32_t oldest_time = 0;
        bool found = false;
        for (size_t i = 0; i < state->entities.length; i++) {
            if (state->entities.data[i].update != update_corpse) continue;
            corpses++;
            EnemyData *ed = (EnemyData *)state->entities.data[i].entity_data;
            if (!found || ed->death_time < oldest_time) {
                found = true;
                oldest_time = ed->death_time;
                oldest_idx = i;
            }
        }
        if (corpses <= MAX_CORPSES) break;
        free(state->entities.data[oldest_idx].entity_data);
        da_remove_unordered(&state->entities, oldest_idx);
    }
}

void spawn_enemies(GameState *state) {
    int count = 0;
    for (size_t i = 0; i < state->entities.length; i++) {
        if (state->entities.data[i].update == update_enemy) count++;
    }
    if (count >= MAX_ENEMIES) return;
    if (state->game_time - last_spawn_time < SPAWN_INTERVAL_MS) return;

    uint8_t *map = state->map;
    for (int attempt = 0; attempt < 32; attempt++) {
        // Sample directly in the ring [MIN..MAX] around the player.
        float ang = (rand() / (float)RAND_MAX) * 2.0f * PI;
        float r = MIN_SPAWN_DIST + (rand() / (float)RAND_MAX) * (MAX_SPAWN_DIST - MIN_SPAWN_DIST);
        int cx = (int)(state->player.pos.x + cosf(ang) * r);
        int cy = (int)(state->player.pos.y + sinf(ang) * r);
        if (cx < 0 || cx >= MAP_COLS || cy < 0 || cy >= MAP_ROWS) continue;
        if (map[cy * MAP_COLS + cx] != 0) continue;
        Vector2 cell_center = {cx + 0.5f, cy + 0.5f};

        EnemyData *ed = malloc(sizeof(EnemyData));
        ed->hp = ENEMY_HP;
        ed->hit_flash_until = 0;
        ed->dead = false;
        ed->death_time = 0;
        da_append(&state->entities, create_enemy_pos(cell_center, ed));
        last_spawn_time = state->game_time;
        cull_corpses(state);
        break;
    }
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

// Abstract bullet icons in the bottom-right: filled = loaded, dim = spent.
void draw_ammo(GameState *state) {
    if (playerState.gun == 0) return;

    int u = state->screen_width / 100;
    if (u < 2) u = 2;
    int bw = 2 * u;                 // bullet width
    int bh = 5 * u;                 // bullet height
    int gap = 2 * u;                // gap between bullets
    int margin = 3 * u;
    int total = GUN_AMMO * bw + (GUN_AMMO - 1) * gap;
    int x0 = state->screen_width - margin - total;
    int y0 = state->screen_height - margin - bh;

    int tipw = bw / 2;
    if (tipw < 1) tipw = 1;
    int tiph = bh / 3;

    for (int i = 0; i < GUN_AMMO; i++) {
        bool loaded = i < playerState.ammo;
        pixel_t body = loaded ? C_YELLOW : color_brightness(C_YELLOW, -0.78f);
        pixel_t tip  = loaded ? C_ORANGE : color_brightness(C_ORANGE, -0.78f);
        int x = x0 + i * (bw + gap);
        draw_rectangle(x + (bw - tipw) / 2, y0, tipw, tiph, tip);   // pointed tip
        draw_rectangle(x, y0 + tiph, bw, bh - tiph, body);          // casing
    }
}

void draw_hud(GameState *state) {
    char buf[64];

    pixel_t hp_color = playerState.hp <= 30 ? C_RED : C_GREEN;
    sprintf(buf, "HP: %d", playerState.hp);
    draw_text(buf, 10, 15, fonts[FONT_SM], hp_color);

    sprintf(buf, "Kills: %d", playerState.kills);
    draw_text(buf, 10, 30, fonts[FONT_SM], C_WHITE);

    if (playerState.gun == 1) {
        const int gun_size = SCREEN_W * GUN_SCALE;
        draw_asset(assets_map[tx_wep_gun0],
                   (state->screen_width - gun_size) / 2,
                   state->screen_height - gun_size,
                   gun_size, gun_size, 128);

        // Muzzle flash: layered bright blobs at the barrel tip, just above the gun.
        if (state->game_time < muzzle_flash_until) {
            int u = state->screen_width / 40;
            if (u < 2) u = 2;
            int fx = state->screen_width / 2;
            int fy = state->screen_height - gun_size + u;
            draw_rectangle(fx - 3 * u, fy - 2 * u, 6 * u, 4 * u, C_ORANGE);
            draw_rectangle(fx - 2 * u, fy - u,     4 * u, 2 * u, C_YELLOW);
            draw_rectangle(fx - u,     fy - u / 2, 2 * u, u,     C_WHITE);
        }
    }

    draw_ammo(state);

    // Out-of-ammo prompt, blinking in the center.
    if (playerState.gun == 1 && playerState.ammo <= 0 && (state->game_time / 350) % 2 == 0) {
        int tx = state->screen_width / 2 - 60;
        int ty = state->screen_height / 2 - 50;
        draw_text("OUT OF AMMO", tx, ty, fonts[FONT_SM], C_RED);
        draw_text("R TO RELOAD", tx, ty + 16, fonts[FONT_SM], C_WHITE);
    }

    int cx = state->screen_width / 2;
    int cy = state->screen_height / 2;
    draw_rectangle(cx - 2, cy, 4, 1, C_WHITE);
    draw_rectangle(cx, cy - 2, 1, 4, C_WHITE);
}

void start_run(GameState *state) {
    for (size_t i = 0; i < state->entities.length; i++) {
        Entity *e = &state->entities.data[i];
        if (e->update == update_enemy || e->update == update_corpse) {
            free(e->entity_data);
        }
    }
    state->entities.length = 0;
    muzzle_flash_until = 0;

    state->player = init_player();
    playerState.hp = 100;
    playerState.ammo = GUN_AMMO;
    playerState.kills = 0;
    playerState.start_time = state->game_time;
    playerState.last_hit_time = 0;
    playerState.survived_ms = 0;
    last_spawn_time = 0;

    level_append_exported_entities(&state->entities);
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

    playerState.hp = 100;
    playerState.gun = 0;
    playerState.ammo = 0;
    playerState.kills = 0;
    playerState.start_time = 0;
    playerState.survived_ms = 0;
    playerState.last_hit_time = 0;

    level_append_exported_entities(&state->entities);

    joystick_init(32, 36, axes);
}

int game_state = 0;
void update_game(GameState *state) {
    if (game_state == 0) {
        draw_text("Press W to start", 120, 30, fonts[FONT_MD], 0xFFFFFF);
        if (is_key_pressed(YARI_KEY_W)) {
            start_run(state);
            game_state = 1;
        }
    } else if (game_state == 1) {
        player_shoot(state);
        spawn_enemies(state);
        draw_game();
        draw_hud(state);
        move_player(state);
        if (playerState.hp <= 0) {
            playerState.survived_ms = state->game_time - playerState.start_time;
            game_state = 2;
        }
    } else {
        char buf[64];
        int cx = SCREEN_W / 2 - 60;
        draw_text("GAME OVER", cx, SCREEN_H / 2 - 40, fonts[FONT_MD], C_RED);
        sprintf(buf, "Kills: %d", playerState.kills);
        draw_text(buf, cx, SCREEN_H / 2 - 10, fonts[FONT_SM], C_WHITE);
        sprintf(buf, "Survived: %.1fs", playerState.survived_ms / 1000.0f);
        draw_text(buf, cx, SCREEN_H / 2 + 10, fonts[FONT_SM], C_WHITE);
        draw_text("Press W to restart", cx, SCREEN_H / 2 + 35, fonts[FONT_SM], C_WHITE);
        if (is_key_pressed(YARI_KEY_W)) {
            start_run(state);
            game_state = 1;
        }
    }
}
