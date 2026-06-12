#include "renderer.h"

/**
 *
 * @param asset pointer to pixel data (top-left origin)
 * @param x x position of the top-left corner
 * @param y y position of the top-left corner
 * @param width desired width of the asset on screen in pixels
 * @param height desired height of the asset on screen in pixels
 * @param row_stride width of the asset in pixels
 */
void draw_asset(
    const pixel_t* asset,
    const int x,
    const int y,
    const int width,
    const int height,
    const int row_stride
) {
    for (int dst_row = 0; dst_row < height; dst_row++) {
        int src_row = dst_row * row_stride / height;

        for (int dst_col = 0; dst_col < width; dst_col++) {
            int src_col = dst_col * row_stride / width;
            pixel_t pixel = asset[src_row * row_stride + src_col];

            if (pixel != EMPTY_PIXEL) {
                draw_rectangle(x + dst_col, y + dst_row, 1, 1, pixel);
            }
        }
    }
}

/**
 *
 * @param text null-terminated string to draw
 * @param x x position of the top-left corner
 * @param y y position of the top-left corner
 * @param font font to use for rendering
 * @param c color to use for rendering
 */
void draw_text(
    const char* text,
    const int x,
    const int y,
    const font_t* font,
    const pixel_t c
) {
    int cursor_x = x;
    for (const char *p = text; *p; p++) {
        char ch = *p;
        if (ch < 32 || ch > 127) continue;

        glyph_t g = font->glyphs[(unsigned char)ch - 32];
        int dst_x = cursor_x + (int)g.xoff;
        int dst_y = y + (int)g.yoff;

        for (int gy = g.y0; gy < g.y1; gy++) {
            for (int gx = g.x0; gx < g.x1; gx++) {
                int i = gy * font->atlas_w + gx;
                if ((font->atlas[i >> 3] >> (i & 7)) & 1)
                    draw_rectangle(dst_x + (gx - g.x0), dst_y + (gy - g.y0), 1, 1, c);
            }
        }
        cursor_x += (int)g.xadvance;
    }
}
