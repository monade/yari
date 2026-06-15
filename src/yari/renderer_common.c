#include "renderer.h"

void yr_draw_texture(
    int x,
    int y,
    int width,
    int height,
    const yr_pixel_t *texture,
    int texture_width,
    int texture_height,
    bool skip_empty
) {
    if (width <= 0 || height <= 0 ||
        texture_width <= 0 || texture_height <= 0 ||
        !texture) {
        return;
    }

    for (int dst_row = 0; dst_row < height; dst_row++) {
        int src_row = dst_row * texture_height / height;

        for (int dst_col = 0; dst_col < width; dst_col++) {
            int src_col = dst_col * texture_width / width;

            yr_pixel_t pixel = texture[src_row * texture_width + src_col];

            if (!skip_empty || pixel != YR_EMPTY_PIXEL) {
                yr_draw_rectangle(x + dst_col, y + dst_row, 1, 1, pixel);
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
void yr_draw_text(
    const char* text,
    const int x,
    const int y,
    const yr_font_t* font,
    const yr_pixel_t c
) {
    int cursor_x = x;
    for (const char *p = text; *p; p++) {
        char ch = *p;
        if (ch < 32 || ch > 127) continue;

        yr_glyph_t g = font->glyphs[(unsigned char)ch - 32];
        int dst_x = cursor_x + (int)g.xoff;
        int dst_y = y + (int)g.yoff;

        for (int gy = g.y0; gy < g.y1; gy++) {
            for (int gx = g.x0; gx < g.x1; gx++) {
                int i = gy * font->atlas_w + gx;
                if ((font->atlas[i >> 3] >> (i & 7)) & 1)
                    yr_draw_rectangle(dst_x + (gx - g.x0), dst_y + (gy - g.y0), 1, 1, c);
            }
        }
        cursor_x += (int)g.xadvance;
    }
}
