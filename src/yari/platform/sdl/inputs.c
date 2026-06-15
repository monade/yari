#include "../../inputs.h"

#include <SDL.h>
#include <stdbool.h>
#include <string.h>

#define YR_SDL_KEY_COUNT 512

static bool key_down[YR_SDL_KEY_COUNT];
static bool key_pressed[YR_SDL_KEY_COUNT];

static bool is_valid_key(int key) {
    return key >= 0 && key < YR_SDL_KEY_COUNT;
}

static int map_sdl_key(SDL_Keycode key) {
    if (key >= SDLK_a && key <= SDLK_z) return YR_KEY_A + (int)(key - SDLK_a);
    if (key >= SDLK_0 && key <= SDLK_9) return YR_KEY_ZERO + (int)(key - SDLK_0);

    switch (key) {
        case SDLK_QUOTE: return YR_KEY_APOSTROPHE;
        case SDLK_COMMA: return YR_KEY_COMMA;
        case SDLK_MINUS: return YR_KEY_MINUS;
        case SDLK_PERIOD: return YR_KEY_PERIOD;
        case SDLK_SLASH: return YR_KEY_SLASH;
        case SDLK_SEMICOLON: return YR_KEY_SEMICOLON;
        case SDLK_EQUALS: return YR_KEY_EQUAL;
        case SDLK_LEFTBRACKET: return YR_KEY_LEFT_BRACKET;
        case SDLK_BACKSLASH: return YR_KEY_BACKSLASH;
        case SDLK_RIGHTBRACKET: return YR_KEY_RIGHT_BRACKET;
        case SDLK_BACKQUOTE: return YR_KEY_GRAVE;
        case SDLK_SPACE: return YR_KEY_SPACE;
        case SDLK_ESCAPE: return YR_KEY_ESCAPE;
        case SDLK_RETURN: return YR_KEY_ENTER;
        case SDLK_TAB: return YR_KEY_TAB;
        case SDLK_BACKSPACE: return YR_KEY_BACKSPACE;
        case SDLK_INSERT: return YR_KEY_INSERT;
        case SDLK_DELETE: return YR_KEY_DELETE;
        case SDLK_RIGHT: return YR_KEY_RIGHT;
        case SDLK_LEFT: return YR_KEY_LEFT;
        case SDLK_DOWN: return YR_KEY_DOWN;
        case SDLK_UP: return YR_KEY_UP;
        case SDLK_PAGEUP: return YR_KEY_PAGE_UP;
        case SDLK_PAGEDOWN: return YR_KEY_PAGE_DOWN;
        case SDLK_HOME: return YR_KEY_HOME;
        case SDLK_END: return YR_KEY_END;
        case SDLK_CAPSLOCK: return YR_KEY_CAPS_LOCK;
        case SDLK_SCROLLLOCK: return YR_KEY_SCROLL_LOCK;
        case SDLK_NUMLOCKCLEAR: return YR_KEY_NUM_LOCK;
        case SDLK_PRINTSCREEN: return YR_KEY_PRINT_SCREEN;
        case SDLK_PAUSE: return YR_KEY_PAUSE;
        case SDLK_F1: return YR_KEY_F1;
        case SDLK_F2: return YR_KEY_F2;
        case SDLK_F3: return YR_KEY_F3;
        case SDLK_F4: return YR_KEY_F4;
        case SDLK_F5: return YR_KEY_F5;
        case SDLK_F6: return YR_KEY_F6;
        case SDLK_F7: return YR_KEY_F7;
        case SDLK_F8: return YR_KEY_F8;
        case SDLK_F9: return YR_KEY_F9;
        case SDLK_F10: return YR_KEY_F10;
        case SDLK_F11: return YR_KEY_F11;
        case SDLK_F12: return YR_KEY_F12;
        case SDLK_LSHIFT: return YR_KEY_LEFT_SHIFT;
        case SDLK_LCTRL: return YR_KEY_LEFT_CONTROL;
        case SDLK_LALT: return YR_KEY_LEFT_ALT;
        case SDLK_LGUI: return YR_KEY_LEFT_SUPER;
        case SDLK_RSHIFT: return YR_KEY_RIGHT_SHIFT;
        case SDLK_RCTRL: return YR_KEY_RIGHT_CONTROL;
        case SDLK_RALT: return YR_KEY_RIGHT_ALT;
        case SDLK_RGUI: return YR_KEY_RIGHT_SUPER;
        default: return YR_KEY_NULL;
    }
}

static bool process_events(void) {
    bool should_quit = false;
    SDL_Event event;

    memset(key_pressed, 0, sizeof(key_pressed));

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            should_quit = true;
            continue;
        }

        if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP) continue;

        int key = map_sdl_key(event.key.keysym.sym);
        if (!is_valid_key(key) || key == YR_KEY_NULL) continue;

        if (event.type == SDL_KEYDOWN) {
            if (!event.key.repeat && !key_down[key]) key_pressed[key] = true;
            key_down[key] = true;
            if (key == YR_KEY_ESCAPE) should_quit = true;
        } else {
            key_down[key] = false;
        }
    }

    return should_quit;
}

bool yr_game_should_close(void) {
    return process_events();
}

void yr_inputs_init(void) {
    memset(key_down, 0, sizeof(key_down));
    memset(key_pressed, 0, sizeof(key_pressed));
}

void yr_esp_key_init(int pin, int key) {
    (void)pin;
    (void)key;
}

int yr_esp_joystick_init(int joystick_pin_x, int joystick_pin_y) {
    (void)joystick_pin_x;
    (void)joystick_pin_y;
    return -1;
}

float yr_esp_joystick_get_axis(int joystick_id, int axis) {
    (void)joystick_id;
    (void)axis;
    return 0.0f;
}

bool yr_is_key_down(int key) {
    return is_valid_key(key) && key_down[key];
}

bool yr_is_key_up(int key) {
    return !yr_is_key_down(key);
}

bool yr_is_key_pressed(int key) {
    return is_valid_key(key) && key_pressed[key];
}
