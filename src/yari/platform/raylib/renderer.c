#include "../../renderer.h"
#include <raylib.h>

static Image frame_buffer;
static Texture2D frame_texture;

float yr_get_frame_time() {
    return GetFrameTime();
}

void yr_draw_rectangle(int x, int y, int width, int height, yr_pixel_t color) {
    ImageDrawRectangle(&frame_buffer, x, y, width, height, GetColor(color));
}

void yr_clear_screen(yr_pixel_t color) {
    ImageClearBackground(&frame_buffer, GetColor(color));
}

void yr_renderer_init(int width, int height, const char *title, unsigned int target_fps) {
    InitWindow(width, height, title);
    if(target_fps > 0) SetTargetFPS(target_fps);
    SetTraceLogLevel(LOG_WARNING);
    frame_buffer = GenImageColor(width, height, BLACK);
    frame_texture = LoadTextureFromImage(frame_buffer);
}

bool yr_game_should_close() {
    return WindowShouldClose();
}

void yr_begin_drawing() {
    BeginDrawing();
}
  
void yr_render_screen() {
    UpdateTexture(frame_texture, frame_buffer.data);
    DrawTexture(frame_texture, 0, 0, WHITE);
}

void yr_end_drawing() {
    EndDrawing();
}

float yr_get_time() {
    return (float)GetTime();
}

float yr_get_fps() {
    return (float)GetFPS();
}
