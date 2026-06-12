#ifndef YR_RENDERER_H
#define YR_RENDERER_H
#include <stdbool.h>
#include "colors.h"

#define YR_FOV_ANGLE (PI / 3.5)
#define YR_MAX_RENDER_DIST 20.0
#define YR_TEXTURE_SIZE 64

typedef enum {
    YR_FONT_SM,
    YR_FONT_MD,
    YR_FONT_LG,
    YR_FONT_XL
} yr_font_size_t;

// Mirrors stbtt_bakedchar: one entry per ASCII character baked into the atlas.
typedef struct {
    short x0, y0, x1, y1;  // atlas pixel bounds of the glyph
    float xoff, yoff;       // offset from cursor to top-left of glyph
    float xadvance;         // how far to advance the cursor after this glyph
} yr_glyph_t;

typedef struct {
    const uint8_t *atlas;   // 1-bit packed bitmap, LSB-first: pixel i → bit i%8 of byte i/8
    const yr_glyph_t *glyphs;  // glyph info for ASCII 32–127 (96 entries)
    int atlas_w, atlas_h;
} yr_font_t;

/**
 * Draw an asset (texture or color) on the screen at the specified position.
 * @param asset      pixel data (top-left origin)
 * @param x          x position of the top-left corner
 * @param y          y position of the top-left corner
 * @param width      desired width of the asset on screen in pixels
 * @param height     desired height of the asset on screen in pixels
 * @param row_stride width of the asset in pixels
 */
void yr_draw_asset(const yr_pixel_t* asset, int x, int y, int width, int height, int row_stride);

/**
 * Draw text on the screen using a baked bitmap font.
 * @param text      null-terminated string to draw
 * @param x         x position of the top-left corner
 * @param y         y position of the top-left corner
 * @param font      font to use (e.g. press_start_2p_sm or press_start_2p_md)
 * @param c         text color
 */
void yr_draw_text(const char *text, int x, int y, const yr_font_t *font, yr_pixel_t c);

float yr_get_frame_time();

float yr_get_time();

void yr_clear_screen(yr_pixel_t color);

void yr_draw_rectangle(int x, int y, int width, int height, yr_pixel_t color);

void yr_renderer_init(int width, int height, const char *title, unsigned int target_fps);

bool yr_game_should_close();

void yr_begin_drawing();

void yr_render_screen();

void yr_end_drawing();

float yr_get_fps();

#ifdef YARI_NO_PREFIX
#define draw_asset yr_draw_asset
#define draw_text yr_draw_text
#define get_frame_time yr_get_frame_time
#define get_time yr_get_time
#define clear_screen yr_clear_screen
#define draw_rectangle yr_draw_rectangle
#define renderer_init yr_renderer_init
#define game_should_close yr_game_should_close
#define begin_drawing yr_begin_drawing
#define render_screen yr_render_screen
#define end_drawing yr_end_drawing
#define get_fps yr_get_fps
#endif

#endif // YR_RENDERER_H
