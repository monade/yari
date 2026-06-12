#ifndef RENDERER_H
#define RENDERER_H
#include <stdbool.h>
#include "colors.h"

#define FOV_ANGLE (PI / 3.5)
#define MAX_RENDER_DIST 20.0
#define TEXTURE_SIZE 64

typedef enum {
    FONT_SM,
    FONT_MD,
    FONT_LG,
    FONT_XL
} font_size_t;

// Mirrors stbtt_bakedchar: one entry per ASCII character baked into the atlas.
typedef struct {
    short x0, y0, x1, y1;  // atlas pixel bounds of the glyph
    float xoff, yoff;       // offset from cursor to top-left of glyph
    float xadvance;         // how far to advance the cursor after this glyph
} glyph_t;

typedef struct {
    const uint8_t *atlas;   // 1-bit packed bitmap, LSB-first: pixel i → bit i%8 of byte i/8
    const glyph_t *glyphs;  // glyph info for ASCII 32–127 (96 entries)
    int atlas_w, atlas_h;
} font_t;

/**
 * Draw an asset (texture or color) on the screen at the specified position.
 * @param asset      pixel data (top-left origin)
 * @param x          x position of the top-left corner
 * @param y          y position of the top-left corner
 * @param width      desired width of the asset on screen in pixels
 * @param height     desired height of the asset on screen in pixels
 * @param row_stride width of the asset in pixels
 */
void draw_asset(const pixel_t* asset, int x, int y, int width, int height, int row_stride);

/**
 * Draw text on the screen using a baked bitmap font.
 * @param text      null-terminated string to draw
 * @param x         x position of the top-left corner
 * @param y         y position of the top-left corner
 * @param font      font to use (e.g. press_start_2p_sm or press_start_2p_md)
 * @param c         text color
 */
void draw_text(const char *text, int x, int y, const font_t *font, pixel_t c);

float get_frame_time();

float get_time();

void draw_rectangle(int x, int y, int width, int height, pixel_t color);

void renderer_init(int width, int height, const char *title, unsigned int target_fps);

bool game_should_close();

void begin_drawing();

void render_screen();

void end_drawing();

float get_fps();

#endif // RENDERER_H