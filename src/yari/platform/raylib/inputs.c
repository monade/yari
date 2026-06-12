#include <raylib.h>
#include "../../inputs.h"
#include <stdbool.h>

bool yr_is_key_down(int key) {
    return IsKeyDown(key);
}

bool yr_is_key_up(int key) {
    return IsKeyUp(key);
}

bool yr_is_key_pressed(int key) {
    return IsKeyPressed(key);
}

void yr_inputs_init() {}

void yr_joystick_init(int joystick_pin_x, int joystick_pin_y, YrJoystickConfig axes[2]) {
  (void)joystick_pin_x;
  (void)joystick_pin_y;
  (void)axes;
}
float yr_joystick_get_axis(YrJoystickConfig axis) {
  (void)axis;
  return 0;
}
