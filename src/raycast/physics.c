
#include "physics.h"
#include "da.h"

static const float RAY_EPSILON = 0.0001f;

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

static CollisionInfo check_wall_ray_collision(GameState *state, Vector2 origin, Vector2 dir, float max_dist, float *hit_dist) {
    CollisionInfo info = {0};

    int map_x = (int)floorf(origin.x);
    int map_y = (int)floorf(origin.y);

    info = check_wall_at(state, map_x, map_y);
    if (info.type != COLLISION_NONE) {
        *hit_dist = 0.0f;
        return info;
    }

    if (fabsf(dir.x) < RAY_EPSILON) dir.x = (dir.x < 0.0f) ? -RAY_EPSILON : RAY_EPSILON;
    if (fabsf(dir.y) < RAY_EPSILON) dir.y = (dir.y < 0.0f) ? -RAY_EPSILON : RAY_EPSILON;

    float delta_x = fabsf(1.0f / dir.x);
    float delta_y = fabsf(1.0f / dir.y);
    float side_dist_x;
    float side_dist_y;
    int step_x;
    int step_y;

    if (dir.x < 0.0f) {
        step_x = -1;
        side_dist_x = (origin.x - (float)map_x) * delta_x;
    } else {
        step_x = 1;
        side_dist_x = ((float)map_x + 1.0f - origin.x) * delta_x;
    }

    if (dir.y < 0.0f) {
        step_y = -1;
        side_dist_y = (origin.y - (float)map_y) * delta_y;
    } else {
        step_y = 1;
        side_dist_y = ((float)map_y + 1.0f - origin.y) * delta_y;
    }

    while (true) {
        if (side_dist_x < side_dist_y) {
            if (side_dist_x > max_dist) break;
            map_x += step_x;
            *hit_dist = side_dist_x;
            side_dist_x += delta_x;
        } else {
            if (side_dist_y > max_dist) break;
            map_y += step_y;
            *hit_dist = side_dist_y;
            side_dist_y += delta_y;
        }

        info = check_wall_at(state, map_x, map_y);
        if (info.type != COLLISION_NONE) return info;

        if (map_x < 0 || map_y < 0 ||
            map_x >= state->map_cols || map_y >= state->map_rows) {
            break;
        }
    }

    return (CollisionInfo){0};
}

CollisionInfo check_collision(GameState *state, Vector2 next_pos, float threshold, uint32_t collision_mask) {
    CollisionInfo info = {0};

    // entities
    if (collision_mask & ~CMSK_WALL) {
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

CollisionInfo check_ray_collision(GameState *state, Vector2 origin, Vector2 dir, float threshold, uint32_t collision_mask) {
    CollisionInfo info = {0};
    if (threshold <= 0.0f) return info;

    float dir_len = Vector2Length(dir);
    if (dir_len < RAY_EPSILON) return info;
    dir = Vector2Scale(dir, 1.0f / dir_len);

    float best_dist = threshold;
    bool has_hit = false;

    if (collision_mask & ~CMSK_WALL) {
        for (size_t i = 0; i < state->entities.length; i++) {
            Entity *sprite = &state->entities.data[i];
            if (sprite->disabled) continue;
            if (!(sprite->collision_mask & collision_mask)) continue;

            float radius = sprite->collision_threshold;
            if (radius < 0.0f) radius = 0.0f;

            Vector2 rel = Vector2Subtract(sprite->pos, origin);
            float projection = Vector2DotProduct(rel, dir);
            float rel_len_sqr = Vector2LengthSqr(rel);
            float closest_dist_sqr = rel_len_sqr - projection * projection;
            if (closest_dist_sqr < 0.0f) closest_dist_sqr = 0.0f;

            float radius_sqr = radius * radius;
            if (closest_dist_sqr > radius_sqr) continue;

            float offset = sqrtf(radius_sqr - closest_dist_sqr);
            float hit_dist = projection - offset;
            if (hit_dist < 0.0f) {
                if (projection + offset < 0.0f) continue;
                hit_dist = 0.0f;
            }

            if (hit_dist <= threshold && (!has_hit || hit_dist < best_dist)) {
                info.type = COLLISION_ENTITY;
                info.entity = sprite;
                info.entity_index = i;
                best_dist = hit_dist;
                has_hit = true;
            }
        }
    }

    if (collision_mask & CMSK_WALL) {
        float wall_dist = 0.0f;
        CollisionInfo wall = check_wall_ray_collision(state, origin, dir, threshold, &wall_dist);
        if (wall.type != COLLISION_NONE && (!has_hit || wall_dist < best_dist)) {
            info = wall;
        }
    }

    return info;
}

Vector2 slide_collision(GameState *state, Vector2 from, Vector2 to, CollisionInfo *hit, float threshold, uint32_t collision_mask) {
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
