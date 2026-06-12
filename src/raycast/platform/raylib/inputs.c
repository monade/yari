#include <raylib.h>
#include "../../inputs.h"
#include <stdbool.h>

bool is_key_down(int key) {
    return IsKeyDown(key);
}

bool is_key_up(int key) {
    return IsKeyUp(key);
}

bool is_key_pressed(int key) {
    return IsKeyPressed(key);
}

void inputs_init() {}

void joystick_init(int joystick_pin_x, int joystick_pin_y, JoystickConfig axes[2]) {
  (void)joystick_pin_x;
  (void)joystick_pin_y;
  (void)axes;
}
float joystick_get_axis(JoystickConfig axis) {
  (void)axis;
  return 0;
}