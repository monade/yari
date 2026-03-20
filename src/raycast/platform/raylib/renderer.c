#include "../../renderer.h"
#include <raylib.h>

static Image frame_buffer;
static Texture2D frame_texture;

float get_frame_time() {
    return GetFrameTime();
}

void draw_rectangle(int x, int y, int width, int height, pixel_t color) {
    ImageDrawRectangle(&frame_buffer, x, y, width, height, GetColor(color));
}

void renderer_init(int width, int height, const char *title, unsigned int target_fps) {
    InitWindow(width, height, title);
    SetTargetFPS(target_fps);
    SetTraceLogLevel(LOG_WARNING);
    frame_buffer = GenImageColor(width, height, BLACK);
    frame_texture = LoadTextureFromImage(frame_buffer);
}

bool game_should_close() {
    return WindowShouldClose();
}

void begin_drawing() {
    BeginDrawing();
}

void end_drawing() {
    UpdateTexture(frame_texture, frame_buffer.data);
    DrawTexture(frame_texture, 0, 0, WHITE);
    EndDrawing();
}

int get_time() {
    return GetTime();
}
