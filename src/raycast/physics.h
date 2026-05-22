#ifndef PHYSICS_H
#define PHYSICS_H
#include "raycast.h"

enum MovementDirection {
  FORWARD,
  BACK,
  LEFT,
  RIGHT
};

enum RotationDirection {
  CLOCKWISE,
  COUNTERCLOCKWISE
};

typedef enum {
  COLLISION_NONE = 0,
  COLLISION_WALL,
  COLLISION_ENTITY
} CollisionType;

typedef struct {
  CollisionType type;
  int cell_x;
  int cell_y;
  uint8_t tile;
  Entity *entity;
  size_t entity_index;
} CollisionInfo;

CollisionInfo check_collision(GameState *state, Vector2 next_pos, float threshold, uint32_t collision_mask);

Vector2 slide_player(GameState *state, Vector2 from, Vector2 to, CollisionInfo *hit, uint32_t collision_mask);

Vector2 rotate(Vector2 vector, enum RotationDirection direction, float rotation_speed);

Vector2 move(Vector2 subject_position, Vector2 subject_direction, enum MovementDirection direction, float movement_speed);

#endif // PHYSICS_H