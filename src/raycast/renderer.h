#ifndef RENDERER_H
#define RENDERER_H
#include <stdbool.h>
#include "colors.h"

#define FOV_ANGLE (PI / 3.5)
#define MAX_RENDER_DIST 20.0
#define TEXTURE_SIZE 64

float get_frame_time();

float get_time();

void draw_rectangle(int x, int y, int width, int height, pixel_t color);

void renderer_init(int width, int height, const char *title, unsigned int target_fps);

bool game_should_close();

void begin_drawing();

void render_screen();

void end_drawing();

void draw_text(const char* text, int x, int y, int font_size, pixel_t c);

float get_fps();

#endif // RENDERER_H