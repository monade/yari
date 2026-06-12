#ifndef INPUTS_H
#define INPUTS_H

#include <stdbool.h>

typedef enum {
    YARI_KEY_NULL            = 0,        // Key: NULL, used for no key pressed
    // Alphanumeric keys
    YARI_KEY_APOSTROPHE      = 39,       // Key: '
    YARI_KEY_COMMA           = 44,       // Key: ,
    YARI_KEY_MINUS           = 45,       // Key: -
    YARI_KEY_PERIOD          = 46,       // Key: .
    YARI_KEY_SLASH           = 47,       // Key: /
    YARI_KEY_ZERO            = 48,       // Key: 0
    YARI_KEY_ONE             = 49,       // Key: 1
    YARI_KEY_TWO             = 50,       // Key: 2
    YARI_KEY_THREE           = 51,       // Key: 3
    YARI_KEY_FOUR            = 52,       // Key: 4
    YARI_KEY_FIVE            = 53,       // Key: 5
    YARI_KEY_SIX             = 54,       // Key: 6
    YARI_KEY_SEVEN           = 55,       // Key: 7
    YARI_KEY_EIGHT           = 56,       // Key: 8
    YARI_KEY_NINE            = 57,       // Key: 9
    YARI_KEY_SEMICOLON       = 59,       // Key: ;
    YARI_KEY_EQUAL           = 61,       // Key: =
    YARI_KEY_A               = 65,       // Key: A | a
    YARI_KEY_B               = 66,       // Key: B | b
    YARI_KEY_C               = 67,       // Key: C | c
    YARI_KEY_D               = 68,       // Key: D | d
    YARI_KEY_E               = 69,       // Key: E | e
    YARI_KEY_F               = 70,       // Key: F | f
    YARI_KEY_G               = 71,       // Key: G | g
    YARI_KEY_H               = 72,       // Key: H | h
    YARI_KEY_I               = 73,       // Key: I | i
    YARI_KEY_J               = 74,       // Key: J | j
    YARI_KEY_K               = 75,       // Key: K | k
    YARI_KEY_L               = 76,       // Key: L | l
    YARI_KEY_M               = 77,       // Key: M | m
    YARI_KEY_N               = 78,       // Key: N | n
    YARI_KEY_O               = 79,       // Key: O | o
    YARI_KEY_P               = 80,       // Key: P | p
    YARI_KEY_Q               = 81,       // Key: Q | q
    YARI_KEY_R               = 82,       // Key: R | r
    YARI_KEY_S               = 83,       // Key: S | s
    YARI_KEY_T               = 84,       // Key: T | t
    YARI_KEY_U               = 85,       // Key: U | u
    YARI_KEY_V               = 86,       // Key: V | v
    YARI_KEY_W               = 87,       // Key: W | w
    YARI_KEY_X               = 88,       // Key: X | x
    YARI_KEY_Y               = 89,       // Key: Y | y
    YARI_KEY_Z               = 90,       // Key: Z | z
    YARI_KEY_LEFT_BRACKET    = 91,       // Key: [
    YARI_KEY_BACKSLASH       = 92,       // Key: '\'
    YARI_KEY_RIGHT_BRACKET   = 93,       // Key: ]
    YARI_KEY_GRAVE           = 96,       // Key: `
    // Function keys
    YARI_KEY_SPACE           = 32,       // Key: Space
    YARI_KEY_ESCAPE          = 256,      // Key: Esc
    YARI_KEY_ENTER           = 257,      // Key: Enter
    YARI_KEY_TAB             = 258,      // Key: Tab
    YARI_KEY_BACKSPACE       = 259,      // Key: Backspace
    YARI_KEY_INSERT          = 260,      // Key: Ins
    YARI_KEY_DELETE          = 261,      // Key: Del
    YARI_KEY_RIGHT           = 262,      // Key: Cursor right
    YARI_KEY_LEFT            = 263,      // Key: Cursor left
    YARI_KEY_DOWN            = 264,      // Key: Cursor down
    YARI_KEY_UP              = 265,      // Key: Cursor up
    YARI_KEY_PAGE_UP         = 266,      // Key: Page up
    YARI_KEY_PAGE_DOWN       = 267,      // Key: Page down
    YARI_KEY_HOME            = 268,      // Key: Home
    YARI_KEY_END             = 269,      // Key: End
    YARI_KEY_CAPS_LOCK       = 280,      // Key: Caps lock
    YARI_KEY_SCROLL_LOCK     = 281,      // Key: Scroll down
    YARI_KEY_NUM_LOCK        = 282,      // Key: Num lock
    YARI_KEY_PRINT_SCREEN    = 283,      // Key: Print screen
    YARI_KEY_PAUSE           = 284,      // Key: Pause
    YARI_KEY_F1              = 290,      // Key: F1
    YARI_KEY_F2              = 291,      // Key: F2
    YARI_KEY_F3              = 292,      // Key: F3
    YARI_KEY_F4              = 293,      // Key: F4
    YARI_KEY_F5              = 294,      // Key: F5
    YARI_KEY_F6              = 295,      // Key: F6
    YARI_KEY_F7              = 296,      // Key: F7
    YARI_KEY_F8              = 297,      // Key: F8
    YARI_KEY_F9              = 298,      // Key: F9
    YARI_KEY_F10             = 299,      // Key: F10
    YARI_KEY_F11             = 300,      // Key: F11
    YARI_KEY_F12             = 301,      // Key: F12
    YARI_KEY_LEFT_SHIFT      = 340,      // Key: Shift left
    YARI_KEY_LEFT_CONTROL    = 341,      // Key: Control left
    YARI_KEY_LEFT_ALT        = 342,      // Key: Alt left
    YARI_KEY_LEFT_SUPER      = 343,      // Key: Super left
    YARI_KEY_RIGHT_SHIFT     = 344,      // Key: Shift right
    YARI_KEY_RIGHT_CONTROL   = 345,      // Key: Control right
    YARI_KEY_RIGHT_ALT       = 346,      // Key: Alt right
    YARI_KEY_RIGHT_SUPER     = 347,      // Key: Super right
} YariKeyboardKey;

typedef enum {
    X_AXIS = 0,
    Y_AXIS = 1,
} JoystickAxis;

#ifdef ESP32
#include "esp_adc/adc_oneshot.h"
typedef struct joystick_axis_t {
  adc_oneshot_unit_handle_t adc_handle;
  adc_unit_t adc_unit;
  adc_channel_t adc_channel;
  int pin;
} JoystickConfig;
#else 
typedef struct joystick_axis_t {
  int adc_handle;
  int adc_unit;
  int adc_channel;
  int pin;
} JoystickConfig;
#endif

void inputs_init();


void joystick_init(int joystick_pin_x, int joystick_pin_y, JoystickConfig axes[2]);
float joystick_get_axis(JoystickConfig axis);
bool is_key_down(int key);
bool is_key_up(int key);
bool is_key_pressed(int key);

#endif // INPUTS_H