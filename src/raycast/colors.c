#include "colors.h"

#ifdef COLOR_565
pixel_t color_brightness(pixel_t color, float factor) {
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
#endif

#ifdef COLOR_32
pixel_t color_brightness(pixel_t color, float factor) {
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

pixel_t color_map[] = {
    C_RED,    // 128
    C_GREEN,  // 129
    C_BLUE,   // 130
    C_YELLOW, // 131
    C_PURPLE, // 132
    C_ORANGE, // 133
    C_WHITE   // 134
};