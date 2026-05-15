#include <raylib.h>
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
