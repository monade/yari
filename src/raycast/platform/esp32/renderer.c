#include <stdint.h>
#include <esp_timer.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "renderer.h"
#include "inputs.h"
#include "colors.h"

#ifdef FB_DRAM
#define FB_ATTR static
#else
// Attribute to place framebuffer in IRAM
#define FB_ATTR __attribute__((section(".iram1"))) static
#endif

// Panel size (T-Display active area)
#ifndef LCD_W
#define LCD_W 240
#endif
#ifndef LCD_H
#define LCD_H 136
#endif
#ifndef LCD_X_OFF
#define LCD_X_OFF 40
#endif
#ifndef LCD_Y_OFF
#define LCD_Y_OFF 53
#endif

// T-Display (ST7789) std pins
#ifndef PIN_MOSI
#define PIN_MOSI 19
#endif
#ifndef PIN_CLK
#define PIN_CLK 18
#endif
#ifndef PIN_CS
#define PIN_CS 5
#endif
#ifndef PIN_DC
#define PIN_DC 16
#endif
#ifndef PIN_RST
#define PIN_RST 23
#endif
#ifndef PIN_BL
#define PIN_BL 4
#endif
#ifndef SPI_CLOCK_SPEED
#define SPI_CLOCK_SPEED (80 * 1000 * 1000)
#endif

#define SCREEN_BUFFER_SIZE (LCD_W * LCD_H * 2)



static int64_t last_time_us = 0;
static int64_t frame_start_time_us = 0;
static int target_fps = 30;
static int64_t target_frame_time_us = 1000000 / 30;

FB_ATTR uint16_t framebuffer[LCD_W * LCD_H];

static inline void write_u16_iram(uint16_t *addr, uint16_t val) {
    uintptr_t ptr = (uintptr_t)addr;
    volatile uint32_t *aligned = (uint32_t *)(ptr & ~3);
    
    if (ptr & 2) {
        *aligned = (*aligned & 0x0000FFFF) | (val << 16);
    } else {
        *aligned = (*aligned & 0xFFFF0000) | val;
    }
}

static spi_device_handle_t spi;

static void lcd_cmd(uint8_t cmd) {
    gpio_set_level(PIN_DC, 0);
    spi_transaction_t t = {0};
    t.length = 8;
    t.tx_buffer = &cmd;
    spi_device_polling_transmit(spi, &t);
}

static void lcd_data(const void *data, int len_bytes) {
    if (len_bytes <= 0) return;
    gpio_set_level(PIN_DC, 1);
    spi_transaction_t t = {0};
    t.length = len_bytes * 8;
    t.tx_buffer = data;
    spi_device_polling_transmit(spi, &t);
}

static void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    x0 += LCD_X_OFF;
    x1 += LCD_X_OFF;
    y0 += LCD_Y_OFF;
    y1 += LCD_Y_OFF;

    uint8_t buf[4];

    lcd_cmd(0x2A); // CASET
    buf[0] = x0 >> 8;
    buf[1] = x0 & 0xFF;
    buf[2] = x1 >> 8;
    buf[3] = x1 & 0xFF;
    lcd_data(buf, 4);

    lcd_cmd(0x2B); // RASET
    buf[0] = y0 >> 8;
    buf[1] = y0 & 0xFF;
    buf[2] = y1 >> 8;
    buf[3] = y1 & 0xFF;
    lcd_data(buf, 4);

    lcd_cmd(0x2C); // RAMWR
}


static void lcd_init(void) {
    gpio_set_direction(PIN_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_BL, GPIO_MODE_OUTPUT);

    // Reset
    gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(PIN_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // SPI bus
    spi_bus_config_t bus = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SCREEN_BUFFER_SIZE,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t dev = {
        .clock_speed_hz = SPI_CLOCK_SPEED,
        .mode = 0,
        .spics_io_num = PIN_CS,
        .queue_size = 7,
        .flags = SPI_DEVICE_HALFDUPLEX
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev, &spi));

    // ST7789 init sequence
    lcd_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(150)); // SWRESET
    lcd_cmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(120)); // SLPOUT

    uint8_t colmod = 0x55; // 16-bit
    lcd_cmd(0x3A);
    lcd_data(&colmod, 1);

    // Rotation landscape
    uint8_t madctl = 0x60;
    lcd_cmd(0x36);
    lcd_data(&madctl, 1);

    lcd_cmd(0x21); // INVON
    lcd_cmd(0x13); // NORON
    lcd_cmd(0x29); // DISPON

    gpio_set_level(PIN_BL, 1);
}

void draw_rectangle(int posX, int posY, int width, int height, pixel_t color) {
    if (width <= 0 || height <= 0) return;
    
    // Clipping
    if (posX < 0) {
        width += posX;
        posX = 0;
    }
    if (posY < 0) {
        height += posY;
        posY = 0;
    }
    if (posX + width > LCD_W) width = LCD_W - posX;
    if (posY + height > LCD_H) height = LCD_H - posY;
    if (width <= 0 || height <= 0) return;

    // big endian framebuffer
    color = (color << 8) | (color >> 8);

    for (int y = 0; y < height; y++) {
        int lCD_offset = (posY + y) * LCD_W + posX;
        for (int x = 0; x < width; x++) {
            #ifdef FB_DRAM
            framebuffer[lCD_offset + x] = color;
            #else
            write_u16_iram(&framebuffer[lCD_offset + x], color);
            #endif
        }
    }
}



static void set_target_fps(unsigned int fps) {
    target_fps = fps;
    target_frame_time_us = 1000000 / fps;
}

void renderer_init(int width, int height, const char *title, unsigned int target_fps) {
    (void)width;
    (void)height;
    (void)title;
    lcd_init();
    inputs_init();
    set_target_fps(target_fps);
}

bool game_should_close() {
    return 0;
}

void begin_drawing() {
    frame_start_time_us = esp_timer_get_time();
}

void render_screen() {
  lcd_set_window(0, 0, LCD_W - 1, LCD_H - 1);
  gpio_set_level(PIN_DC, 1);
  
  spi_transaction_t t = {0};
  t.length = SCREEN_BUFFER_SIZE * 8;
  t.tx_buffer = ((uint8_t*)framebuffer);
  
  ESP_ERROR_CHECK(spi_device_transmit(spi, &t));
}

void end_drawing() {
    if (target_fps > 0) {
        int64_t frame_end_time_us = esp_timer_get_time();
        int64_t frame_elapsed_us = frame_end_time_us - frame_start_time_us;
        int64_t sleep_time_us = target_frame_time_us - frame_elapsed_us;
        
        if (sleep_time_us > 0) {
            vTaskDelay(pdMS_TO_TICKS(sleep_time_us / 1000));
        }
    }
}

float get_frame_time() {
    int64_t current_time_us = esp_timer_get_time();
    if (last_time_us == 0)
    {
        last_time_us = current_time_us;
        return 0.0f;
    }

    int64_t delta_us = current_time_us - last_time_us;
    last_time_us = current_time_us;

    return (float)delta_us / 1000000.0f;
}

float get_time() {
    return esp_timer_get_time() / 1000000.0f; // Return time in seconds
}

float get_fps() {
    float frame_time = get_frame_time();
    if (frame_time <= 0.0f) return 0.0f;
    return 1.0f / frame_time;
}