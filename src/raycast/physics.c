
#include "physics.h"
#include "da.h"

static CollisionInfo check_wall_at(GameState *state, int cell_x, int cell_y) {
    CollisionInfo info = {0};

    // out of bounds check
    if (cell_x < 0 || cell_y < 0 ||
        cell_x >= state->map_cols || cell_y >= state->map_rows) {
        info.type = COLLISION_NONE;
        info.cell_x = cell_x;
        info.cell_y = cell_y;
        info.tile = 0;
        return info;
    }
    uint8_t tile = state->map[cell_y * state->map_cols + cell_x];
    if (tile) {
        info.type = COLLISION_WALL;
        info.cell_x = cell_x;
        info.cell_y = cell_y;
        info.tile = tile;
    }
    return info;
}

CollisionInfo check_collision(GameState *state, Vector2 next_pos, float threshold, uint32_t collision_mask) {
    CollisionInfo info = {0};

    // entities
    if (collision_mask & CMSK_ENTITY) {
        for (size_t i = 0; i < state->entities.length; i++) {
            Entity *sprite = &state->entities.data[i];
            if (sprite->disabled) continue;
            if (!(sprite->collision_mask & collision_mask)) continue;
            float dist = Vector2Distance(next_pos, sprite->pos);
            if (dist < sprite->collision_threshold + threshold) {
                info.type = COLLISION_ENTITY;
                info.entity = sprite;
                info.entity_index = i;
                return info;
            }
        }
    }

    // walls
    if (collision_mask & CMSK_WALL) {
        Vector2 samples[] = {
            next_pos,
            { next_pos.x + threshold, next_pos.y },
            { next_pos.x - threshold, next_pos.y },
            { next_pos.x, next_pos.y + threshold },
            { next_pos.x, next_pos.y - threshold },
        };
        for (size_t i = 0; i < ARRAY_LEN(samples); i++) {
            info = check_wall_at(state, (int)samples[i].x, (int)samples[i].y);
            if (info.type != COLLISION_NONE) return info;
        }
    }
    return info;
}

Vector2 slide_player(GameState *state, Vector2 from, Vector2 to, CollisionInfo *hit, uint32_t collision_mask) {
    float threshold = state->player.collision_threshold;
    CollisionInfo info = check_collision(state, to, threshold, collision_mask);
    if (hit) *hit = info;
    if (info.type == COLLISION_NONE) return to;

    Vector2 slide_x = { to.x, from.y };
    if (check_collision(state, slide_x, threshold, collision_mask).type == COLLISION_NONE) {
        return slide_x;
    }
    Vector2 slide_y = { from.x, to.y };
    if (check_collision(state, slide_y, threshold, collision_mask).type == COLLISION_NONE) {
        return slide_y;
    }
    return from;
}

Vector2 rotate(Vector2 vector, enum RotationDirection direction, float rotation_speed) {
    float dt = get_frame_time();
    if (direction == COUNTERCLOCKWISE) {
        dt *= -1;
    }
    return Vector2Rotate(vector, dt * rotation_speed);
}

Vector2 move(Vector2 subject_position, Vector2 subject_direction, enum MovementDirection direction, float movement_speed) {
    float dt = get_frame_time();
    if (direction == BACK) {
        dt *= -1;
    }
    if (direction == LEFT || direction == RIGHT) {
        subject_direction = Vector2Rotate(subject_direction, direction == LEFT ? -PI/2 : PI/2);
    }
    return Vector2Add(subject_position, Vector2Scale(subject_direction, dt*movement_speed));
}