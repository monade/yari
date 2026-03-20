#ifndef COLORS_H
#define COLORS_H
#include <stdint.h>



#ifdef COLOR_565
typedef uint16_t pixel_t;
#define EMPTY_PIXEL 0
#define C_BLACK 0x1
#define C_WHITE 0xFFFF
#define C_RED 0xF800
#define C_GREEN 0x07E0
#define C_BLUE 0x001F
#define C_YELLOW 0xFFE0
#define C_PURPLE 0xF81F
#define C_ORANGE 0xFC00

#elif !defined(COLOR_32)
#define COLOR_32
#endif

#ifdef COLOR_32
typedef uint32_t pixel_t;
#define EMPTY_PIXEL 0xFF
#define C_BLACK 0x000001FF
#define C_WHITE 0xFFFFFFFF
#define C_RED 0xFF0000FF
#define C_GREEN 0x00FF00FF
#define C_BLUE 0x0000FFFF
#define C_YELLOW 0xFFFF00FF
#define C_PURPLE 0xFF00FFFF
#define C_ORANGE 0xFF8000FF
#endif

extern pixel_t color_map[];

pixel_t color_brightness(pixel_t color, float factor) ;

#endif // COLORS_H