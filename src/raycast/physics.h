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

bool check_player_collision(GameState *state, Vector2 next_pos);

Vector2 rotate(Vector2 vector, enum RotationDirection direction, float rotation_speed);

Vector2 move(Vector2 subject_position, Vector2 subject_direction, enum MovementDirection direction, float movement_speed);

#endif // PHYSICS_H