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

void yr_inputs_init() {
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

void yr_joystick_init(int joystick_pin_x, int joystick_pin_y, YrJoystickConfig axes[2]) {
  axes[YR_X_AXIS].pin = joystick_pin_x;
  axes[YR_Y_AXIS].pin = joystick_pin_y;

  adc_oneshot_io_to_channel(joystick_pin_x, &axes[YR_X_AXIS].adc_unit, &axes[YR_X_AXIS].adc_channel);
  adc_oneshot_io_to_channel(joystick_pin_y, &axes[YR_Y_AXIS].adc_unit, &axes[YR_Y_AXIS].adc_channel);

  // 1. Configura l'unità ADC
  adc_oneshot_unit_init_cfg_t unit_cfg_x = {
    .unit_id = axes[YR_X_AXIS].adc_unit,
  };

  adc_oneshot_new_unit(&unit_cfg_x, &axes[YR_X_AXIS].adc_handle);

  if (axes[YR_Y_AXIS].adc_unit == axes[YR_X_AXIS].adc_unit) {
    axes[YR_Y_AXIS].adc_handle = axes[YR_X_AXIS].adc_handle;
  } else {
    adc_oneshot_unit_init_cfg_t unit_cfg_y = { .unit_id = axes[YR_Y_AXIS].adc_unit };
    adc_oneshot_new_unit(&unit_cfg_y, &axes[YR_Y_AXIS].adc_handle);
  }

  // 2. Configura il canale
  adc_oneshot_config_channel(axes[YR_X_AXIS].adc_handle, axes[YR_X_AXIS].adc_channel, &chan_cfg_x);
  adc_oneshot_config_channel(axes[YR_Y_AXIS].adc_handle, axes[YR_Y_AXIS].adc_channel, &chan_cfg_y);
}

bool yr_is_key_down(int key) {
  if (key == YR_KEY_A) {
    return !gpio_get_level(PIN_KEY_A);
  }
  if (key == YR_KEY_D) {
    return !gpio_get_level(PIN_KEY_D);
  }
  if (key == YR_KEY_S) {
    return !gpio_get_level(PIN_KEY_S);
  }
  if (key == YR_KEY_W) {
    return !gpio_get_level(PIN_KEY_W);
  }
  return 0;
}

bool yr_is_key_up(int key) {
  return !yr_is_key_down(key);
}

bool yr_is_key_pressed(int key) {
  bool prev = gpios_states[key];
  bool current = yr_is_key_down(key);

  gpios_states[key] = current;

  return current && !prev;
}

float yr_joystick_get_axis(YrJoystickConfig axis) {
  int value = 0;
  adc_oneshot_read(axis.adc_handle, axis.adc_channel, &value);

  return ((float)value / 4095.0) * 2. - 1.;
}
