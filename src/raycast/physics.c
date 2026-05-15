
#include "physics.h"

bool check_player_collision(GameState *state, Vector2 next_pos) {
    int cell_x = (int)next_pos.x;
    int cell_y = (int)next_pos.y;
    // FIXME: Check map collision
    if(state->map[cell_y * state->map_cols + cell_x]) {
        return true;
    }
    cell_x = (int)(next_pos.x + state->player.collision_threshold);
    if(state->map[cell_y * state->map_cols + cell_x]) {
        return true;
    }
    cell_x = (int)(next_pos.x - state->player.collision_threshold);
    if(state->map[cell_y * state->map_cols + cell_x]) {
        return true;
    }
    cell_y = (int)(next_pos.y + state->player.collision_threshold);
    if(state->map[cell_y * state->map_cols + cell_x]) {
        return true;
    }
    cell_y = (int)(next_pos.y - state->player.collision_threshold);
    if(state->map[cell_y * state->map_cols + cell_x]) {
        return true;
    }
    for(size_t i=0; i<state->entities.length; i++) {
        if(state->entities.data[i].disabled) continue;
        Entity *sprite = &state->entities.data[i];
        if(!sprite->collision_mask) continue;
        if(Vector2Distance(next_pos, sprite->pos) < sprite->collision_threshold + state->player.collision_threshold) {
            return true;
        }
    }
    return false;
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