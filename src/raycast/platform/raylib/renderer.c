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
  
void render_screen() {
    UpdateTexture(frame_texture, frame_buffer.data);
    DrawTexture(frame_texture, 0, 0, WHITE);
}

void end_drawing() {
    EndDrawing();
}

float get_time() {
    return (float)GetTime();
}

void draw_text(const char* text, int x, int y, int font_size, pixel_t c) {
    ImageDrawText(&frame_buffer, text, x, y, font_size, GetColor(c));
}

float get_fps() {
    return (float)GetFPS();
}