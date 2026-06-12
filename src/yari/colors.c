#include "colors.h"

#ifdef COLOR_565
yr_pixel_t yr_color_brightness(yr_pixel_t color, float factor) {
    if (factor > 1.0f) factor = 1.0f;
    else if (factor < -1.0f) factor = -1.0f;

    float red = color >> 11; // 24 per 32
    float green = (color >> 5) & 0x3F; // 111111 // 16 xFF
    float blue =  (color) & 0x1F; // 11111 // 8 xFF

    if (factor < 0.0f)
    {
        factor = 1.0f + factor;
        red *= factor;
        green *= factor;
        blue *= factor;
    }
    else
    {
        red = (31 - red)*factor + red; // 255
        green = (63 - green)*factor + green; // 255
        blue = (31 - blue)*factor + blue; // 255
    }

    return ((int)red << 11) | ((int)green << 5) | (int)blue; // 24, 16, 0 | FF
}
#else
yr_pixel_t yr_color_brightness(yr_pixel_t color, float factor) {
    if (factor > 1.0f) factor = 1.0f;
    else if (factor < -1.0f) factor = -1.0f;

    float red = color >> 24;
    float green = (color >> 16) & 0xFF;
    float blue =  (color >> 8) & 0xFF;

    if (factor < 0.0f)
    {
        factor = 1.0f + factor;
        red *= factor;
        green *= factor;
        blue *= factor;
    }
    else
    {
        red = (255 - red)*factor + red; // 255
        green = (255 - green)*factor + green; // 255
        blue = (255 - blue)*factor + blue; // 255
    }

    return ((int)red << 24) | ((int)green << 16) | ((int)blue << 8) | 0xFF;
}
#endif

yr_pixel_t yr_color_map[] = {
    YR_BLACK,
    YR_WHITE,
    YR_RED,
    YR_GREEN,
    YR_BLUE,
    YR_YELLOW,
    YR_PURPLE,
    YR_ORANGE,
    YR_CYAN,
    YR_PINK,
    YR_GRAY,
    YR_SILVER,
    YR_MAROON,
    YR_DARK_RED,
    YR_DARK_GREEN,
    YR_DARK_BLUE,
    YR_OLIVE,
    YR_TEAL,
    YR_NAVY,
    YR_BROWN,
    YR_SKY_BLUE
};
