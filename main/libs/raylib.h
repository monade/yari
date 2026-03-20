// Raylib shim for ESP32 with ST7789 LCD

/**********************************************************************************************
*
*   raylib v5.6-dev - A simple and easy-to-use library to enjoy videogames programming (www.raylib.com)
*
*   FEATURES:
*       - NO external dependencies, all required libraries included with raylib
*       - Multiplatform: Windows, Linux, FreeBSD, OpenBSD, NetBSD, DragonFly,
*                        MacOS, Haiku, Android, Raspberry Pi, DRM native, HTML5
*       - Written in plain C code (C99) in PascalCase/camelCase notation
*       - Hardware accelerated with OpenGL (1.1, 2.1, 3.3, 4.3, ES2, ES3 - choose at compile)
*       - Unique OpenGL abstraction layer (usable as standalone module): [rlgl]
*       - Multiple Fonts formats supported (TTF, OTF, FNT, BDF, Sprite fonts)
*       - Outstanding texture formats support, including compressed formats (DXT, ETC, ASTC)
*       - Full 3d support for 3d Shapes, Models, Billboards, Heightmaps and more!
*       - Flexible Materials system, supporting classic maps and PBR maps
*       - Animated 3D models supported (skeletal bones animation) (IQM, M3D, GLTF)
*       - Shaders support, including Model shaders and Postprocessing shaders
*       - Powerful math module for Vector, Matrix and Quaternion operations: [raymath]
*       - Audio loading and playing with streaming support (WAV, OGG, MP3, FLAC, QOA, XM, MOD)
*       - VR stereo rendering with configurable HMD device parameters
*       - Bindings to multiple programming languages available!
*
*   NOTES:
*       - One default Font is loaded on InitWindow()->LoadFontDefault() [core, text]
*       - One default Texture2D is loaded on rlglInit(), 1x1 white pixel R8G8B8A8 [rlgl] (OpenGL 3.3 or ES2)
*       - One default Shader is loaded on rlglInit()->rlLoadShaderDefault() [rlgl] (OpenGL 3.3 or ES2)
*       - One default RenderBatch is loaded on rlglInit()->rlLoadRenderBatch() [rlgl] (OpenGL 3.3 or ES2)
*
*   DEPENDENCIES (included):
*       [rcore][GLFW] rglfw (Camilla Löwy - github.com/glfw/glfw) for window/context management and input
*       [rcore][RGFW] rgfw (ColleagueRiley - github.com/ColleagueRiley/RGFW) for window/context management and input
*       [rlgl] glad/glad_gles2 (David Herberth - github.com/Dav1dde/glad) for OpenGL 3.3 extensions loading
*       [raudio] miniaudio (David Reid - github.com/mackron/miniaudio) for audio device/context management
*
*   OPTIONAL DEPENDENCIES (included):
*       [rcore] sinfl (Micha Mettke) for DEFLATE decompression algorithm
*       [rcore] sdefl (Micha Mettke) for DEFLATE compression algorithm
*       [rcore] rprand (Ramon Santamaria) for pseudo-random numbers generation
*       [rtextures] qoi (Dominic Szablewski - https://phoboslab.org) for QOI image manage
*       [rtextures] stb_image (Sean Barret) for images loading (BMP, TGA, PNG, JPEG, HDR...)
*       [rtextures] stb_image_write (Sean Barret) for image writing (BMP, TGA, PNG, JPG)
*       [rtextures] stb_image_resize2 (Sean Barret) for image resizing algorithms
*       [rtextures] stb_perlin (Sean Barret) for Perlin Noise image generation
*       [rtext] stb_truetype (Sean Barret) for ttf fonts loading
*       [rtext] stb_rect_pack (Sean Barret) for rectangles packing
*       [rmodels] par_shapes (Philip Rideout) for parametric 3d shapes generation
*       [rmodels] tinyobj_loader_c (Syoyo Fujita) for models loading (OBJ, MTL)
*       [rmodels] cgltf (Johannes Kuhlmann) for models loading (glTF)
*       [rmodels] m3d (bzt) for models loading (M3D, https://bztsrc.gitlab.io/model3d)
*       [rmodels] vox_loader (Johann Nadalutti) for models loading (VOX)
*       [raudio] dr_wav (David Reid) for WAV audio file loading
*       [raudio] dr_flac (David Reid) for FLAC audio file loading
*       [raudio] dr_mp3 (David Reid) for MP3 audio file loading
*       [raudio] stb_vorbis (Sean Barret) for OGG audio loading
*       [raudio] jar_xm (Joshua Reisenauer) for XM audio module loading
*       [raudio] jar_mod (Joshua Reisenauer) for MOD audio module loading
*       [raudio] qoa (Dominic Szablewski - https://phoboslab.org) for QOA audio manage
*
*
*   LICENSE: zlib/libpng
*
*   raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software:
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef _RAYLIB_H_
#define _RAYLIB_H_

#include <stdint.h>


// =================== CONFIG ===================


// GPIO predefined buttons
// #ifndef PIN_KEY_A
// #define PIN_KEY_A 0
// #endif
// #ifndef PIN_KEY_D
// #define PIN_KEY_D 35
// #endif

// =================== TYPES & STRUCTS ===================
#if !defined(RL_VECTOR2_TYPE)
// Vector2 type
typedef struct Vector2 {
    float x;
    float y;
} Vector2;
#define RL_VECTOR2_TYPE
#endif

#define CLITERAL(type) (type)

typedef struct {
    uint16_t r : 5;
    uint16_t g : 6;
    uint16_t b : 5;
} Color;

static_assert(sizeof(Color) == 2, "Color struct size should be 2 bytes");

#define LIGHTGRAY CLITERAL(Color){25, 50, 25}   // Light Gray
#define GRAY CLITERAL(Color){16, 33, 16}        // Gray
#define DARKGRAY CLITERAL(Color){10, 20, 10}    // Dark Gray
#define YELLOW CLITERAL(Color){31, 62, 0}       // Yellow
#define GOLD CLITERAL(Color){31, 50, 0}         // Gold
#define ORANGE CLITERAL(Color){31, 40, 0}       // Orange
#define PINK CLITERAL(Color){31, 27, 24}        // Pink
#define RED CLITERAL(Color){28, 10, 6}          // Red
#define MAROON CLITERAL(Color){23, 8, 6}        // Maroon
#define GREEN CLITERAL(Color){0, 57, 6}         // Green
#define LIME CLITERAL(Color){0, 39, 5}          // Lime
#define DARKGREEN CLITERAL(Color){0, 29, 5}     // Dark Green
#define SKYBLUE CLITERAL(Color){12, 47, 31}     // Sky Blue
#define BLUE CLITERAL(Color){0, 30, 30}         // Blue
#define DARKBLUE CLITERAL(Color){0, 20, 21}     // Dark Blue
#define PURPLE CLITERAL(Color){24, 30, 31}      // Purple
#define VIOLET CLITERAL(Color){16, 15, 23}      // Violet
#define DARKPURPLE CLITERAL(Color){13, 7, 15}   // Dark Purple
#define BEIGE CLITERAL(Color){26, 43, 16}       // Beige
#define BROWN CLITERAL(Color){15, 26, 9}        // Brown
#define DARKBROWN CLITERAL(Color){9, 15, 5}     // Dark Brown

#define WHITE CLITERAL(Color){31, 63, 31}       // White
#define BLACK CLITERAL(Color){0, 0, 0}          // Black
#define MAGENTA CLITERAL(Color){31, 0, 31}      // Magenta
#define RAYWHITE CLITERAL(Color){30, 61, 30}    // My own White (raylib logo)

typedef enum {
    KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    KEY_APOSTROPHE      = 39,       // Key: '
    KEY_COMMA           = 44,       // Key: ,
    KEY_MINUS           = 45,       // Key: -
    KEY_PERIOD          = 46,       // Key: .
    KEY_SLASH           = 47,       // Key: /
    KEY_ZERO            = 48,       // Key: 0
    KEY_ONE             = 49,       // Key: 1
    KEY_TWO             = 50,       // Key: 2
    KEY_THREE           = 51,       // Key: 3
    KEY_FOUR            = 52,       // Key: 4
    KEY_FIVE            = 53,       // Key: 5
    KEY_SIX             = 54,       // Key: 6
    KEY_SEVEN           = 55,       // Key: 7
    KEY_EIGHT           = 56,       // Key: 8
    KEY_NINE            = 57,       // Key: 9
    KEY_SEMICOLON       = 59,       // Key: ;
    KEY_EQUAL           = 61,       // Key: =
    KEY_A               = 65,       // Key: A | a
    KEY_B               = 66,       // Key: B | b
    KEY_C               = 67,       // Key: C | c
    KEY_D               = 68,       // Key: D | d
    KEY_E               = 69,       // Key: E | e
    KEY_F               = 70,       // Key: F | f
    KEY_G               = 71,       // Key: G | g
    KEY_H               = 72,       // Key: H | h
    KEY_I               = 73,       // Key: I | i
    KEY_J               = 74,       // Key: J | j
    KEY_K               = 75,       // Key: K | k
    KEY_L               = 76,       // Key: L | l
    KEY_M               = 77,       // Key: M | m
    KEY_N               = 78,       // Key: N | n
    KEY_O               = 79,       // Key: O | o
    KEY_P               = 80,       // Key: P | p
    KEY_Q               = 81,       // Key: Q | q
    KEY_R               = 82,       // Key: R | r
    KEY_S               = 83,       // Key: S | s
    KEY_T               = 84,       // Key: T | t
    KEY_U               = 85,       // Key: U | u
    KEY_V               = 86,       // Key: V | v
    KEY_W               = 87,       // Key: W | w
    KEY_X               = 88,       // Key: X | x
    KEY_Y               = 89,       // Key: Y | y
    KEY_Z               = 90,       // Key: Z | z
    KEY_LEFT_BRACKET    = 91,       // Key: [
    KEY_BACKSLASH       = 92,       // Key: '\'
    KEY_RIGHT_BRACKET   = 93,       // Key: ]
    KEY_GRAVE           = 96,       // Key: `
    // Function keys
    KEY_SPACE           = 32,       // Key: Space
    KEY_ESCAPE          = 256,      // Key: Esc
    KEY_ENTER           = 257,      // Key: Enter
    KEY_TAB             = 258,      // Key: Tab
    KEY_BACKSPACE       = 259,      // Key: Backspace
    KEY_INSERT          = 260,      // Key: Ins
    KEY_DELETE          = 261,      // Key: Del
    KEY_RIGHT           = 262,      // Key: Cursor right
    KEY_LEFT            = 263,      // Key: Cursor left
    KEY_DOWN            = 264,      // Key: Cursor down
    KEY_UP              = 265,      // Key: Cursor up
    KEY_PAGE_UP         = 266,      // Key: Page up
    KEY_PAGE_DOWN       = 267,      // Key: Page down
    KEY_HOME            = 268,      // Key: Home
    KEY_END             = 269,      // Key: End
    KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    KEY_NUM_LOCK        = 282,      // Key: Num lock
    KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    KEY_PAUSE           = 284,      // Key: Pause
    KEY_F1              = 290,      // Key: F1
    KEY_F2              = 291,      // Key: F2
    KEY_F3              = 292,      // Key: F3
    KEY_F4              = 293,      // Key: F4
    KEY_F5              = 294,      // Key: F5
    KEY_F6              = 295,      // Key: F6
    KEY_F7              = 296,      // Key: F7
    KEY_F8              = 297,      // Key: F8
    KEY_F9              = 298,      // Key: F9
    KEY_F10             = 299,      // Key: F10
    KEY_F11             = 300,      // Key: F11
    KEY_F12             = 301,      // Key: F12
    KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    KEY_LEFT_CONTROL    = 341,      // Key: Control left
    KEY_LEFT_ALT        = 342,      // Key: Alt left
    KEY_LEFT_SUPER      = 343,      // Key: Super left
    KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    KEY_RIGHT_ALT       = 346,      // Key: Alt right
    KEY_RIGHT_SUPER     = 347,      // Key: Super right
    KEY_KB_MENU         = 348,      // Key: KB menu
    // Keypad keys
    KEY_KP_0            = 320,      // Key: Keypad 0
    KEY_KP_1            = 321,      // Key: Keypad 1
    KEY_KP_2            = 322,      // Key: Keypad 2
    KEY_KP_3            = 323,      // Key: Keypad 3
    KEY_KP_4            = 324,      // Key: Keypad 4
    KEY_KP_5            = 325,      // Key: Keypad 5
    KEY_KP_6            = 326,      // Key: Keypad 6
    KEY_KP_7            = 327,      // Key: Keypad 7
    KEY_KP_8            = 328,      // Key: Keypad 8
    KEY_KP_9            = 329,      // Key: Keypad 9
    KEY_KP_DECIMAL      = 330,      // Key: Keypad .
    KEY_KP_DIVIDE       = 331,      // Key: Keypad /
    KEY_KP_MULTIPLY     = 332,      // Key: Keypad *
    KEY_KP_SUBTRACT     = 333,      // Key: Keypad -
    KEY_KP_ADD          = 334,      // Key: Keypad +
    KEY_KP_ENTER        = 335,      // Key: Keypad Enter
    KEY_KP_EQUAL        = 336,      // Key: Keypad =
    // Android key buttons
    KEY_BACK            = 4,        // Key: Android back button
    KEY_MENU            = 5,        // Key: Android menu button
    KEY_VOLUME_UP       = 24,       // Key: Android volume up button
    KEY_VOLUME_DOWN     = 25        // Key: Android volume down button
} KeyboardKey;


typedef enum {
    MOUSE_BUTTON_LEFT    = 0,       // Mouse button left
    MOUSE_BUTTON_RIGHT   = 1,       // Mouse button right
    MOUSE_BUTTON_MIDDLE  = 2,       // Mouse button middle (pressed wheel)
    MOUSE_BUTTON_SIDE    = 3,       // Mouse button side (advanced mouse device)
    MOUSE_BUTTON_EXTRA   = 4,       // Mouse button extra (advanced mouse device)
    MOUSE_BUTTON_FORWARD = 5,       // Mouse button forward (advanced mouse device)
    MOUSE_BUTTON_BACK    = 6,       // Mouse button back (advanced mouse device)
} MouseButton;

// static int64_t last_time_us = 0;
// static int64_t frame_start_time_us = 0;
// static int target_fps = 30;
// static int64_t target_frame_time_us = 1000000 / 30;

// FB_ATTR uint16_t framebuffer[LCD_W * LCD_H];

// write a 16-bit value to an address in IRAM, handling unaligned accesses
// static inline void write_u16_iram(uint16_t *addr, uint16_t val) {
//     uintptr_t ptr = (uintptr_t)addr;
//     volatile uint32_t *aligned = (uint32_t *)(ptr & ~3);
    
//     if (ptr & 2) {
//         *aligned = (*aligned & 0x0000FFFF) | (val << 16);
//     } else {
//         *aligned = (*aligned & 0xFFFF0000) | val;
//     }
// }

// =================== SPI/LCD Driver ===================



// =================== GPIO button/inputs initialization ===================
// static void inputs_init() {
//     gpio_set_direction(PIN_KEY_A, GPIO_MODE_INPUT);
//     gpio_set_pull_mode(PIN_KEY_A, GPIO_PULLUP_ONLY);
//     gpio_set_direction(PIN_KEY_D, GPIO_MODE_INPUT);
//     gpio_set_pull_mode(PIN_KEY_D, GPIO_PULLUP_ONLY);
// }

// =================== PUBLIC API ===================
// Color GetColor(uint16_t value) {
//     Color c = {
//         .r = (value >> 11) & 31,
//         .g = (value >> 5) & 63,
//         .b = value & 31
//     };
//     return c;
// }

// uint16_t ColorToInt(Color c) {
//     uint16_t uc = (c.r << 11) | (c.g << 5) | (c.b);
//     return (uc >> 8) | (uc << 8);
// }

// void ClearBackground(Color color) {
//     uint16_t rgb565_color = ColorToInt(color);
//     for (int i = 0; i < LCD_W * LCD_H; i++) {
//         #ifdef FB_DRAM
//         framebuffer[i] = rgb565_color;
//         #else
//         write_u16_iram(&framebuffer[i], rgb565_color);
//         #endif
//     }
// }



// int IsKeyDown(KeyboardKey key) {
//     if (key == KEY_W) {
//         return !gpio_get_level(PIN_KEY_D) && !gpio_get_level(PIN_KEY_A);
//     }
//     if (key == KEY_A) {
//         return !gpio_get_level(PIN_KEY_A);
//     }
//     if (key == KEY_D) {
//         return !gpio_get_level(PIN_KEY_D);
//     }
//     return 0;
// }

// Color ColorBrightness(Color color, float factor) {
//     Color result = color;
//
//     if (factor > 1.0f) factor = 1.0f;
//     else if (factor < -1.0f) factor = -1.0f;
//
//     float red = (float)color.r;
//     float green = (float)color.g;
//     float blue = (float)color.b;
//
//     if (factor < 0.0f)
//     {
//         factor = 1.0f + factor;
//         red *= factor;
//         green *= factor;
//         blue *= factor;
//     }
//     else
//     {
//         red = (31 - red)*factor + red;
//         green = (63 - green)*factor + green;
//         blue = (31 - blue)*factor + blue;
//     }
//
//     result.r = (unsigned char)red;
//     result.g = (unsigned char)green;
//     result.b = (unsigned char)blue;
//
//     return result;
// }


// =================== UNUSED STUBS ===================
#define FLAG_MSAA_4X_HINT 0
void SetConfigFlags(int flags) {}
void DrawRectangleLines(int x, int y, int width, int height, Color color) {}
void DrawLine(int startX, int startY, int endX, int endY, Color color) {}
void DrawCircleV(Vector2 center, float radius, Color color) {}
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color) {}

#endif // _RAYLIB_H_