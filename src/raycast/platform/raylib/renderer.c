#include "../../renderer.h"
#include <raylib.h>

float get_frame_time() {
    return GetFrameTime();
}

void draw_rectangle(int x, int y, int width, int height, pixel_t color) {
    DrawRectangle(x, y, width, height, GetColor(color));
}

void renderer_init(int width, int height, const char *title, unsigned int target_fps) {
    InitWindow(width, height, title);
    SetTargetFPS(target_fps);
}

bool game_should_close() {
    return WindowShouldClose();
}

void begin_drawing() {
    BeginDrawing();
}

void end_drawing() {
    EndDrawing();
}

int get_time() {
    return GetTime();
}
