#include <stdbool.h>
#include "../../inputs.h"
#include "driver/gpio.h"

// GPIO predefined buttons
#ifndef PIN_KEY_A
#define PIN_KEY_A 0
#endif
#ifndef PIN_KEY_D
#define PIN_KEY_D 35
#endif


void inputs_init() {
    gpio_set_direction(PIN_KEY_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_KEY_A, GPIO_PULLUP_ONLY);
    gpio_set_direction(PIN_KEY_D, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_KEY_D, GPIO_PULLUP_ONLY);
}


bool is_key_down(int key) {
    if (key == KEY_W) {
        return !gpio_get_level(PIN_KEY_D) && !gpio_get_level(PIN_KEY_A);
    }
    if (key == KEY_A) {
        return !gpio_get_level(PIN_KEY_A);
    }
    if (key == KEY_D) {
        return !gpio_get_level(PIN_KEY_D);
    }
    return 0;
}