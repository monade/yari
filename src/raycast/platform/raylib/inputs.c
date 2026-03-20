#include <raylib.h>
#include <stdbool.h>

bool is_key_down(int key) {
    return IsKeyDown(key);
}

void inputs_init() {}