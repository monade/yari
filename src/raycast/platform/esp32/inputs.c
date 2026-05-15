#include <stdbool.h>
#include "../../inputs.h"
#include "driver/gpio.h"

// GPIO predefined buttons
#ifndef PIN_KEY_A
#define PIN_KEY_A 34
#endif

#ifndef PIN_KEY_D
#define PIN_KEY_D 35
#endif

#ifndef PIN_KEY_S
#define PIN_KEY_S 36
#endif

#ifndef PIN_KEY_W
#define PIN_KEY_W 37
#endif


void inputs_init() {
    gpio_set_direction(PIN_KEY_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_KEY_A, GPIO_PULLUP_ONLY);

    gpio_set_direction(PIN_KEY_D, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_KEY_D, GPIO_PULLUP_ONLY);

    gpio_set_direction(PIN_KEY_S, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_KEY_S, GPIO_PULLUP_ONLY);

    gpio_set_direction(PIN_KEY_W, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_KEY_W, GPIO_PULLUP_ONLY);
}


bool is_key_down(int key) {
    if (key == KEY_A) {
        return !gpio_get_level(PIN_KEY_A);
    }
    if (key == KEY_D) {
        return !gpio_get_level(PIN_KEY_D);
    }
    if (key == KEY_S) {
        return !gpio_get_level(PIN_KEY_S);
    }
    if (key == KEY_W) {
        return gpio_get_level(PIN_KEY_W);
    }
    return 0;
}

bool is_key_up(int key) {
    return !is_key_down(key);
}
