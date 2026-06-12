#ifndef YR_YARI_H
#define YR_YARI_H

#define RAYMATH_STATIC_INLINE
#include <stddef.h>
#include "raymath.h"
#include "renderer.h"
#include "inputs.h"
#include "colors.h"
#include "da.h"

typedef struct {
    Vector2 pos;
    Vector2 dir;
    float collision_threshold;
} YrPlayer;

typedef struct YrGameState YrGameState;

#define YR_CMSK_NONE    0
#define YR_CMSK_WALL    1
#define YR_CMSK_ALL    -1

typedef struct YrEntity {
    Vector2 pos;
    int texture_id;
    float dist;
    float vdiv;
    float hdiv;
    float vmove;
    bool disabled;
    void *entity_data;
    uint32_t collision_mask;
    float collision_threshold;
    void (*update)(YrGameState *state, struct YrEntity *self, size_t index);
} YrEntity;

typedef struct {
    YrEntity *data;
    size_t length;
    size_t capacity;
} YrEntities;

typedef struct YrGameState {
    YrPlayer player;
    int screen_width;
    int screen_height;
    char *game_title;
    unsigned int target_fps;
    uint8_t *map;
    uint8_t map_cols;
    uint8_t map_rows;
    YrEntities entities;
    unsigned int ray_res;
    float *zbuffer;
    const yr_pixel_t **assets_map;
    size_t floor_texture;
    size_t ceil_texture;
    uint32_t game_time; // ms since start of game
    void* state_attributes; // game-defined state
} YrGameState;


void yr_raycast_walls(YrGameState *state, Vector2 dir, int slice_x);

void yr_draw_walls(YrGameState *state);

void yr_draw_background(YrGameState *state);

void yr_draw_entities(YrGameState *state);

void yr_draw_game();

#ifdef WASM
__attribute__((export_name("wasm_init")))
#endif
void _yr_init_game();

void yr_init_game(YrGameState *state);

#ifdef WASM
__attribute__((export_name("wasm_frame")))
#endif
void _yr_update_game();

void yr_update_game(YrGameState *state);

void _yr_free_game();

#include "physics.h"

#ifdef YARI_NO_PREFIX
#define Player YrPlayer
#define Entity YrEntity
#define Entities YrEntities
#define GameState YrGameState
#define raycast_walls yr_raycast_walls
#define draw_walls yr_draw_walls
#define draw_background yr_draw_background
#define draw_entities yr_draw_entities
#define draw_game yr_draw_game
#endif

#endif // YR_YARI_H

#ifdef YARI_MAIN
#undef YARI_MAIN

#ifdef PLATFORM_WEB
#include <emscripten/emscripten.h>
#endif

#ifdef ESP32
int app_main()
#else
int main()
#endif
{
    _yr_init_game();
#ifdef PLATFORM_WEB
    emscripten_set_main_loop(_yr_update_game, 0, 1);
#else
    while (!yr_game_should_close()) {
        _yr_update_game();
    }
#endif
    _yr_free_game();
    return 0;
}

#endif // YARI_MAIN
