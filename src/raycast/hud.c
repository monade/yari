#include "hud.h"
#include "renderer.h"

void draw_asset(pixel_t *asset, int x, int y, int width, int height, int row_stride) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            draw_rectangle(x + col, y + row, 1, 1, asset[row * row_stride + col]);
        }
    }
}

void draw_text(const char *text, int x, int y, font_t font, pixel_t c) {
    int cursor_x = x;
    for (const char *p = text; *p; p++) {
        char ch = *p;
        if (ch < 32 || ch > 127) continue;

        glyph_t g = font.glyphs[(unsigned char)ch - 32];
        int dst_x = cursor_x + (int)g.xoff;
        int dst_y = y + (int)g.yoff;

        for (int gy = g.y0; gy < g.y1; gy++) {
            for (int gx = g.x0; gx < g.x1; gx++) {
                int i = gy * font.atlas_w + gx;
                if ((font.atlas[i >> 3] >> (i & 7)) & 1)
                    draw_rectangle(dst_x + (gx - g.x0), dst_y + (gy - g.y0), 1, 1, c);
            }
        }
        cursor_x += (int)g.xadvance;
    }
}
