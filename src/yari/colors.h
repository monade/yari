#ifndef YR_COLORS_H
#define YR_COLORS_H
#include <stdint.h>



#ifdef COLOR_565
typedef uint16_t yr_pixel_t;
#define YR_EMPTY_PIXEL 0
#define YR_BLACK       0x0001
#define YR_WHITE       0xFFFF
#define YR_RED         0xF800
#define YR_GREEN       0x07E0
#define YR_BLUE        0x001F
#define YR_YELLOW      0xFFE0
#define YR_PURPLE      0xF81F
#define YR_ORANGE      0xFC00
#define YR_CYAN        0x07FF
#define YR_PINK        0xFE19
#define YR_GRAY        0x8410
#define YR_SILVER      0xC618
#define YR_MAROON      0x8000
#define YR_DARK_RED    0x8800
#define YR_DARK_GREEN  0x0400
#define YR_DARK_BLUE   0x0011
#define YR_OLIVE       0x8400
#define YR_TEAL        0x0410
#define YR_NAVY        0x0010
#define YR_BROWN       0xA145
#define YR_SKY_BLUE    0x865D
#else
typedef uint32_t yr_pixel_t;
#define YR_EMPTY_PIXEL 0xFF
#define YR_BLACK       0x000001FF
#define YR_WHITE       0xFFFFFFFF
#define YR_RED         0xFF0000FF
#define YR_GREEN       0x00FF00FF
#define YR_BLUE        0x0000FFFF
#define YR_YELLOW      0xFFFF00FF
#define YR_PURPLE      0xFF00FFFF
#define YR_ORANGE      0xFF8000FF
#define YR_CYAN        0x00FFFFFF
#define YR_PINK        0xFFC0CBFF
#define YR_GRAY        0x808080FF
#define YR_SILVER      0xC0C0C0FF
#define YR_MAROON      0x800000FF
#define YR_DARK_RED    0x8B0000FF
#define YR_DARK_GREEN  0x008000FF
#define YR_DARK_BLUE   0x00008BFF
#define YR_OLIVE       0x808000FF
#define YR_TEAL        0x008080FF
#define YR_NAVY        0x000080FF
#define YR_BROWN       0xA52A2AFF
#define YR_SKY_BLUE    0x87CEEBFF
#endif

extern yr_pixel_t yr_color_map[];

yr_pixel_t yr_color_brightness(yr_pixel_t color, float factor) ;

#ifdef YARI_NO_PREFIX
#define color_brightness yr_color_brightness
#endif

#endif // YR_COLORS_H
