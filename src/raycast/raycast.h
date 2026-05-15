#ifndef RAYCAST_H
#define RAYCAST_H

#define RAYMATH_STATIC_INLINE
#include <stddef.h>
#include "raymath.h"
#include "renderer.h"
#include "inputs.h"
#include "colors.h"

typedef struct {
    Vector2 pos;
    Vector2 dir;
    float collision_threshold;
} Player;

typedef struct GameState GameState;

typedef struct Entity {
    Vector2 pos;
    int texture_id;
    float dist;
    float vdiv;
    float hdiv;
    float vmove;
    bool disabled;
    void *data;
    uint32_t collision_mask;
    float collision_threshold;
    void (*update)(GameState *state, struct Entity *self);
} Entity;

typedef struct GameState {
    Player player;
    int screen_width;
    int screen_height;
    char *game_title;
    unsigned int target_fps;
    uint8_t *map;
    uint8_t map_cols;
    uint8_t map_rows;
    Entity *entities;
    size_t num_entities;
    unsigned int ray_res;
    float *zbuffer;
    const pixel_t **assets_map;
    size_t floor_texture;
    size_t ceil_texture;
} GameState;



void raycast_walls(GameState *state, Vector2 dir, int slice_x);

void draw_walls(GameState *state);

void draw_background(GameState *state);

void draw_entities(GameState *state);

void draw_game();

#ifdef WASM
__attribute__((export_name("wasm_init")))
#endif
void _init_game();

void init_game(GameState *state);

#ifdef WASM
__attribute__((export_name("wasm_frame")))
#endif
void _update_game();

void update_game(GameState *state);

void _free_game();

#include "physics.h"

#endif // RAYCAST_H

#ifdef RAYCAST_MAIN
#undef RAYCAST_MAIN

#ifdef ESP32
int app_main()
#else
int main()
#endif
{
    _init_game();
    while (!game_should_close()) {
        _update_game();
    }
    _free_game();
    return 0;
}

#endif // RAYCAST_MAIN
