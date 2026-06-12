#include <stdbool.h>
#include "../../inputs.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

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

bool gpios_states[350] = {0};

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

adc_oneshot_chan_cfg_t chan_cfg_x = {
  .atten = ADC_ATTEN_DB_12, // range 0–3.3V
  .bitwidth = ADC_BITWIDTH_12, // 0–4095
};
adc_oneshot_chan_cfg_t chan_cfg_y = {
  .atten = ADC_ATTEN_DB_12, // range 0–3.3V
  .bitwidth = ADC_BITWIDTH_12, // 0–4095
};

void joystick_init(int joystick_pin_x, int joystick_pin_y, JoystickConfig axes[2]) {
  JoystickConfig* axis_x = &axes[0];
  JoystickConfig* axis_y = &axes[1];

  axis_x->pin = joystick_pin_x;
  axis_y->pin = joystick_pin_y;

  adc_oneshot_io_to_channel(joystick_pin_x, &axis_x->adc_unit, &axis_x->adc_channel);
  adc_oneshot_io_to_channel(joystick_pin_y, &axis_y->adc_unit, &axis_y->adc_channel);

  // 1. Configura l'unità ADC
  adc_oneshot_unit_init_cfg_t unit_cfg_x = {
    .unit_id = axis_x->adc_unit,
  };

  adc_oneshot_new_unit(&unit_cfg_x, &axis_x->adc_handle);

  if (axis_y->adc_unit == axis_x->adc_unit) {
    axis_y->adc_handle = axis_x->adc_handle;
  } else {
    adc_oneshot_unit_init_cfg_t unit_cfg_y = { .unit_id = axis_y->adc_unit };
    adc_oneshot_new_unit(&unit_cfg_y, &axis_y->adc_handle);
  }

  // 2. Configura il canale
  adc_oneshot_config_channel(axis_x->adc_handle, axis_x->adc_channel, &chan_cfg_x);
  adc_oneshot_config_channel(axis_y->adc_handle, axis_y->adc_channel, &chan_cfg_y);
}

bool is_key_down(int key) {
  if (key == YARI_KEY_A) {
    return !gpio_get_level(PIN_KEY_A);
  }
  if (key == YARI_KEY_D) {
    return !gpio_get_level(PIN_KEY_D);
  }
  if (key == YARI_KEY_S) {
    return !gpio_get_level(PIN_KEY_S);
  }
  if (key == YARI_KEY_W) {
    return !gpio_get_level(PIN_KEY_W);
  }
  return 0;
}

bool is_key_up(int key) {
  return !is_key_down(key);
}

bool is_key_pressed(int key) {
  bool prev = gpios_states[key];
  bool current = is_key_down(key);

  gpios_states[key] = current;

  return current && !prev;
}

float joystick_get_axis(JoystickConfig axis) {
  int value = 0;
  adc_oneshot_read(axis.adc_handle, axis.adc_channel, &value);

  return ((float)value / 4095.0) * 2. - 1.;
}
