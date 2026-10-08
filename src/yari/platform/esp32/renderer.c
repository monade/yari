#include <stdint.h>
#include <string.h>
#include <esp_attr.h>
#include <esp_timer.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_io_spi.h"
#include "renderer.h"
#include "colors.h"

// Panel active-area offset (gap between the controller's GRAM origin and the
// visible glass, e.g. 40/53 on the T-Display's ST7789)
#ifndef LCD_X_OFF
#define LCD_X_OFF 40
#endif
#ifndef LCD_Y_OFF
#define LCD_Y_OFF 53
#endif

// SPI pins
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
// Backlight is optional: define PIN_BL to -1 (or just don't drive it) for
// panels whose backlight isn't GPIO-controlled.
#ifndef PIN_BL
#define PIN_BL 4
#endif
#ifndef LCD_BL_ACTIVE_LOW
#define LCD_BL_ACTIVE_LOW false
#endif

#ifndef LCD_SPI_HOST
#define LCD_SPI_HOST SPI2_HOST
#endif
#ifndef LCD_SPI_MODE
#define LCD_SPI_MODE 0
#endif

// Controller selection. Add another esp_lcd-compatible driver by defining
// its own LCD_CONTROLLER_xxx and wiring it into lcd_init()/LCD_BITS_PER_PIXEL
// below - everything else in this file (bus setup, rotation, framebuffer,
// blit/pack) is controller-agnostic.
#if !defined(LCD_CONTROLLER_ST7789) && !defined(LCD_CONTROLLER_SSD1306)
#define LCD_CONTROLLER_ST7789
#endif

#ifndef SPI_CLOCK_SPEED
#if defined(LCD_CONTROLLER_SSD1306)
#define SPI_CLOCK_SPEED (10 * 1000 * 1000)
#else
#define SPI_CLOCK_SPEED (80 * 1000 * 1000)
#endif
#endif

// Rotation/mirroring, applied through esp_lcd's portable panel ops so it
// works the same way regardless of which controller is selected. Defaults
// reproduce this project's original T-Display (ST7789) landscape orientation.
#ifndef LCD_MIRROR_X
#define LCD_MIRROR_X true
#endif
#ifndef LCD_MIRROR_Y
#define LCD_MIRROR_Y false
#endif
#ifndef LCD_SWAP_XY
#define LCD_SWAP_XY true
#endif
#ifndef LCD_BGR_ORDER
#define LCD_BGR_ORDER false
#endif
#ifndef LCD_INVERT_COLOR
// Most small ST7789 modules need INVON to show correct colors; other
// controllers default to the non-inverted, natural reading.
#if defined(LCD_CONTROLLER_ST7789)
#define LCD_INVERT_COLOR true
#else
#define LCD_INVERT_COLOR false
#endif
#endif

// The engine renders at YR_LCD_W x YR_LCD_H and the panel shows it LCD_SCALE
// times larger in each direction (pixel replication while converting), so a
// big panel needs a framebuffer of only its render size.
#ifndef LCD_SCALE
#define LCD_SCALE 1
#endif
#define LCD_PANEL_W (YR_LCD_W * LCD_SCALE)
#define LCD_PANEL_H (YR_LCD_H * LCD_SCALE)

// Panel color depth on the wire: 1 = monochrome (Bayer-dithered, packed per
// the controller's native GDDRAM layout), 16 = RGB565, 18 = RGB666 (3 bytes
// per pixel). The engine's pixel format (RGB565 or L8, see colors.h) is
// converted to it once per frame in lcd_present(). RGB565 on a 16bpp panel
// at LCD_SCALE 1 is the zero-copy path: the framebuffer is blitted as is.
// YR_MONOCROME selects the 1bpp wire path even without a monochrome
// controller selected above.
#ifndef LCD_BITS_PER_PIXEL
#if defined(LCD_CONTROLLER_SSD1306) || defined(YR_MONOCROME)
#define LCD_BITS_PER_PIXEL 1
#else
#define LCD_BITS_PER_PIXEL 16
#endif
#endif
#if LCD_BITS_PER_PIXEL != 1 && LCD_BITS_PER_PIXEL != 16 && LCD_BITS_PER_PIXEL != 18
#error "LCD_BITS_PER_PIXEL must be 1, 16 or 18"
#endif

#if defined(YR_RGB565) && LCD_BITS_PER_PIXEL == 16 && LCD_SCALE == 1
#define LCD_ZERO_COPY
#endif

#define LCD_BYTES_PER_PIXEL ((LCD_BITS_PER_PIXEL + 7) / 8)

// 16/18bpp conversion goes out in bands of LCD_BAND_ROWS panel rows (a
// multiple of LCD_SCALE) through two small buffers, so the wire buffers stay a
// few KB whatever the panel size.
#ifndef LCD_BAND_ROWS
#define LCD_BAND_ROWS (16 / LCD_SCALE * LCD_SCALE)
#endif
#if LCD_BAND_ROWS < LCD_SCALE || LCD_BAND_ROWS % LCD_SCALE != 0
#error "LCD_BAND_ROWS must be a positive multiple of LCD_SCALE"
#endif
// Rounded up to 4 bytes so both buffers stay word-aligned for the SPI DMA.
#define LCD_BAND_BYTES ((LCD_PANEL_W * LCD_BAND_ROWS * LCD_BYTES_PER_PIXEL + 3) & ~3)

// LCD_TRANSFERS is the number of draw_bitmap calls (and so of
// on_color_trans_done callbacks) per frame; LCD_WIRE_BUFFER_SIZE is the
// largest one.
#if defined(LCD_ZERO_COPY)
#define LCD_TRANSFERS 1
#define LCD_WIRE_BUFFER_SIZE (LCD_PANEL_W * LCD_PANEL_H * LCD_BYTES_PER_PIXEL)
#elif LCD_BITS_PER_PIXEL == 1
#define LCD_TRANSFERS 1
#define LCD_WIRE_BUFFER_SIZE (((LCD_PANEL_H + 7) / 8) * LCD_PANEL_W)
#else
#define LCD_TRANSFERS ((LCD_PANEL_H + LCD_BAND_ROWS - 1) / LCD_BAND_ROWS)
#define LCD_WIRE_BUFFER_SIZE LCD_BAND_BYTES
#endif

#define SCREEN_PIXEL_COUNT (YR_LCD_W * YR_LCD_H)

static int64_t last_frame_start_us = 0;
static int64_t frame_start_time_us = 0;
static float cached_frame_time = 0.0f;
static int target_fps = 30;
static int64_t target_frame_time_us = 1000000 / 30;

// Framebuffer the engine draws into: one uint16_t slot per pixel, holding an
// RGB565 or L8 value (see colors.h). Its byte order matches the wire format
// only in the zero-copy path (see lcd_color()); every other panel format is
// produced from this buffer once per frame in lcd_present().
static uint16_t framebuffer0[SCREEN_PIXEL_COUNT];

static uint16_t *fb_back = framebuffer0;

// Engine pixel to 8-bit channels. px_luma uses the Rec. 601 weights of
// YR_COLOR in the L8 format.
#ifdef YR_RGB565
static inline uint8_t px_r(uint16_t c) { uint8_t v = c >> 11; return (v << 3) | (v >> 2); }
static inline uint8_t px_g(uint16_t c) { uint8_t v = (c >> 5) & 0x3F; return (v << 2) | (v >> 4); }
static inline uint8_t px_b(uint16_t c) { uint8_t v = c & 0x1F; return (v << 3) | (v >> 2); }
static inline uint8_t px_luma(uint16_t c) { return (77 * px_r(c) + 150 * px_g(c) + 29 * px_b(c)) >> 8; }
#else // YR_L8
static inline uint8_t px_r(uint16_t c) { return (uint8_t)c; }
static inline uint8_t px_g(uint16_t c) { return (uint8_t)c; }
static inline uint8_t px_b(uint16_t c) { return (uint8_t)c; }
static inline uint8_t px_luma(uint16_t c) { return (uint8_t)c; }
#endif

// Engine pixel as an RGB565 word (L8 becomes gray).
#ifdef YR_RGB565
static inline uint16_t px_rgb565(uint16_t c) { return c; }
#else
static inline uint16_t px_rgb565(uint16_t c) { return ((c >> 3) << 11) | ((c >> 2) << 5) | (c >> 3); }
#endif

// RGB565 word in the panel's byte order.
static inline uint16_t lcd_swap16(uint16_t c) {
#ifdef ESP32_DISPLAY_LITTLE_ENDIAN
    return c;
#else
    return (uint16_t)((c << 8) | (c >> 8));
#endif
}

// In the zero-copy path the framebuffer stores panel-order (byte-swapped)
// RGB565 pixels so a frame can be blitted verbatim with no per-frame
// conversion pass; other panel formats are converted once per frame instead
// (see lcd_present()), so storage there is the engine pixel as is.
static inline uint16_t lcd_color(yr_pixel_t color) {
#ifdef LCD_ZERO_COPY
    return lcd_swap16((uint16_t)color);
#else
    return (uint16_t)color;
#endif
}

static inline void fill_pixels(uint16_t *dst, int count, uint16_t color) {
    if (count <= 0) return;

    if (((uintptr_t)dst & 2) != 0) {
        *dst++ = color;
        count--;
    }

    uint32_t color2 = (uint32_t)color | ((uint32_t)color << 16);
    uint32_t *dst32 = (uint32_t *)dst;

    while (count >= 2) {
        *dst32++ = color2;
        count -= 2;
    }

    if (count > 0) {
        *(uint16_t *)dst32 = color;
    }
}

static esp_lcd_panel_io_handle_t lcd_io;
static esp_lcd_panel_handle_t lcd_panel;
static SemaphoreHandle_t lcd_trans_done_sem;

// Runs in the SPI driver's ISR context each time a draw_bitmap's color data has
// been fully clocked out, so yr_render_screen() can block until the whole frame
// has been sent.
static bool YR_PERF_ATTR lcd_on_color_trans_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    (void)io;
    (void)edata;
    BaseType_t high_task_awoken = pdFALSE;
    xSemaphoreGiveFromISR((SemaphoreHandle_t)user_ctx, &high_task_awoken);
    return high_task_awoken == pdTRUE;
}

static void lcd_init(void) {
#if PIN_BL >= 0
    gpio_set_direction(PIN_BL, GPIO_MODE_OUTPUT);
#endif

    spi_bus_config_t bus = {
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = PIN_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIRE_BUFFER_SIZE,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(LCD_SPI_HOST, &bus, SPI_DMA_CH_AUTO));

    lcd_trans_done_sem = xSemaphoreCreateCounting(LCD_TRANSFERS, 0);

    esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = PIN_CS,
        .dc_gpio_num = PIN_DC,
        .spi_mode = LCD_SPI_MODE,
        .pclk_hz = SPI_CLOCK_SPEED,
        .trans_queue_depth = 1,
        .on_color_trans_done = lcd_on_color_trans_done,
        .user_ctx = lcd_trans_done_sem,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_SPI_HOST, &io_config, &lcd_io));

#if defined(LCD_CONTROLLER_SSD1306)
    esp_lcd_panel_ssd1306_config_t ssd1306_config = { .height = LCD_PANEL_H };
#endif
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_RST,
        .rgb_ele_order = LCD_BGR_ORDER ? LCD_RGB_ELEMENT_ORDER_BGR : LCD_RGB_ELEMENT_ORDER_RGB,
        #ifdef ESP32_DISPLAY_LITTLE_ENDIAN
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        #else
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        #endif
        .bits_per_pixel = LCD_BITS_PER_PIXEL,
        #if defined(LCD_CONTROLLER_SSD1306)
        .vendor_config = &ssd1306_config,
        #endif
    };

#if defined(LCD_CONTROLLER_SSD1306)
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(lcd_io, &panel_config, &lcd_panel));
#else
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(lcd_io, &panel_config, &lcd_panel));
    // To support another esp_lcd-compatible controller (e.g. NT35510, or a
    // managed component such as ILI9341/GC9A01), add another #elif branch
    // here calling its esp_lcd_new_panel_xxx() constructor.
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel));

    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd_panel, LCD_MIRROR_X, LCD_MIRROR_Y));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd_panel, LCD_SWAP_XY));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(lcd_panel, LCD_X_OFF, LCD_Y_OFF));

    if (LCD_INVERT_COLOR) {
        ESP_ERROR_CHECK(esp_lcd_panel_invert_color(lcd_panel, true));
    }

    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel, true));

#if PIN_BL >= 0
    gpio_set_level(PIN_BL, LCD_BL_ACTIVE_LOW ? 0 : 1);
#endif
}

#ifdef LCD_ZERO_COPY
// Fast path: the framebuffer already stores panel-ready RGB565 words (see
// lcd_color()), so a frame is just blitted verbatim with no conversion pass.
static void lcd_present(void) {
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(lcd_panel, 0, 0, LCD_PANEL_W, LCD_PANEL_H, fb_back));
}
#elif LCD_BITS_PER_PIXEL == 1
static uint8_t lcd_wire_buffer[LCD_WIRE_BUFFER_SIZE];

// Packs the framebuffer into the SSD1306/SH110x-style page-major 1bpp GDDRAM
// layout: each byte holds 8 vertically-stacked pixels (LSB = topmost row of
// its page), laid out page-by-page. Pixels go through the shared Bayer
// dither (see colors.h), the same one the YR_MONOCROME desktop simulation uses.
static void lcd_present(void) {
    memset(lcd_wire_buffer, 0, sizeof(lcd_wire_buffer));
    for (int y = 0; y < LCD_PANEL_H; y++) {
        uint8_t *page = lcd_wire_buffer + (y / 8) * LCD_PANEL_W;
        uint8_t bit = (uint8_t)(1 << (y % 8));
        const uint16_t *src = fb_back + (y / LCD_SCALE) * YR_LCD_W;
        for (int x = 0; x < LCD_PANEL_W; x++) {
            if (yr_mono_dither_lit(px_luma(src[x / LCD_SCALE]), x, y)) page[x] |= bit;
        }
    }
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(lcd_panel, 0, 0, LCD_PANEL_W, LCD_PANEL_H, lcd_wire_buffer));
}
#else
static uint8_t lcd_band_buffer[2][LCD_BAND_BYTES] __attribute__((aligned(4)));

// Appends the wire bytes of one engine pixel: RGB565 in panel byte order for
// 16bpp, RGB666 for 18bpp (3 bytes, each channel in the 6 high bits of its
// byte, expanded with the high bits replicated into the low ones so white
// stays full scale).
static inline uint8_t *lcd_emit(uint8_t *dst, uint16_t c) {
#if LCD_BITS_PER_PIXEL == 18
    *dst++ = px_r(c);
    *dst++ = px_g(c);
    *dst++ = px_b(c);
#else
    uint16_t word = lcd_swap16(px_rgb565(c));
    memcpy(dst, &word, sizeof(word));
    dst += sizeof(word);
#endif
    return dst;
}

// Converts panel rows [y, y + rows) into buf. Each framebuffer row is
// converted once, with every pixel emitted LCD_SCALE times, then copied for
// the other LCD_SCALE - 1 panel rows it covers.
static void lcd_fill_band(uint8_t *buf, int y, int rows) {
    const size_t row_bytes = LCD_PANEL_W * LCD_BYTES_PER_PIXEL;
    for (int r = 0; r < rows; r += LCD_SCALE) {
        const uint16_t *src = fb_back + ((y + r) / LCD_SCALE) * YR_LCD_W;
        uint8_t *row = buf + r * row_bytes;
        uint8_t *dst = row;
        for (int x = 0; x < YR_LCD_W; x++) {
            uint8_t *first = dst;
            dst = lcd_emit(dst, src[x]);
            for (int s = 1; s < LCD_SCALE; s++, dst += LCD_BYTES_PER_PIXEL) {
                memcpy(dst, first, LCD_BYTES_PER_PIXEL);
            }
        }
        for (int k = 1; k < LCD_SCALE; k++) memcpy(row + k * row_bytes, row, row_bytes);
    }
}

// Converts and sends the frame band by band, alternating two buffers. Each
// draw_bitmap first waits for the transfer still in flight (esp_lcd's SPI io
// drains its queue before sending the window commands), so by the time band
// b + 2 is converted into a buffer, band b has already left it, and the
// conversion of one band overlaps the DMA of the previous one.
static void lcd_present(void) {
    for (int band = 0, y = 0; y < LCD_PANEL_H; band++, y += LCD_BAND_ROWS) {
        int rows = LCD_PANEL_H - y < LCD_BAND_ROWS ? LCD_PANEL_H - y : LCD_BAND_ROWS;
        uint8_t *buf = lcd_band_buffer[band & 1];
        lcd_fill_band(buf, y, rows);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(lcd_panel, 0, y, LCD_PANEL_W, y + rows, buf));
    }
}
#endif

int yr_screen_width(void) {
    return YR_LCD_W;
}

int yr_screen_height(void) {
    return YR_LCD_H;
}

// Precondition (guaranteed by yr_draw_rectangle in renderer_common.c): the
// whole [x, x+width) run at row y is in bounds. width == 1 skips
// fill_pixels's 32-bit batching, not worth it for a single pixel.
void YR_PERF_ATTR yr_fill_span(int x, int y, int width, yr_pixel_t color) {
    uint16_t *dst = fb_back + y * YR_LCD_W + x;
    if (width == 1) {
        *dst = lcd_color(color);
        return;
    }
    fill_pixels(dst, width, lcd_color(color));
}

void YR_PERF_ATTR yr_clear_screen(yr_pixel_t color) {
    fill_pixels(fb_back, SCREEN_PIXEL_COUNT, lcd_color(color));
}

typedef struct {
    YrColorFilterCallback apply;
    void *user_data;
} yr_filter_job_ctx;

// Applies the filter to framebuffer rows [y_start, y_end). With
// YR_MULTITHREAD this runs concurrently on both cores over disjoint row
// ranges, so the callback must be safe to call from either core.
static void YR_PERF_ATTR yr_filter_rows(void *arg, int y_start, int y_end) {
    const yr_filter_job_ctx *ctx = (const yr_filter_job_ctx *)arg;

    uint16_t *px = fb_back + y_start * YR_LCD_W;
    for (int y = y_start; y < y_end; y++) {
        for (int x = 0; x < YR_LCD_W; x++, px++) {
            // In the zero-copy path the framebuffer holds panel-order
            // (byte-swapped) pixels; the swap is its own inverse, so lcd_color
            // converts in both directions.
            yr_pixel_t color = (yr_pixel_t)lcd_color(*px);
            ctx->apply(x, y, &color, ctx->user_data);
            *px = lcd_color(color);
        }
    }
}

void YR_PERF_ATTR yr_apply_color_filter(YrColorFilterCallback apply, void *user_data) {
    if (!apply) return;

    yr_filter_job_ctx ctx = { .apply = apply, .user_data = user_data };
#ifdef YR_MULTITHREAD
    yr_run_split(yr_filter_rows, &ctx, YR_LCD_H);
#else
    yr_filter_rows(&ctx, 0, YR_LCD_H);
#endif
}

yr_pixel_t *get_framebuffer() {
    return (yr_pixel_t *)fb_back;
}


static void set_target_fps(unsigned int fps) {
    target_fps = fps;
    target_frame_time_us = fps > 0 ? 1000000 / fps : 0;
}

void yr_renderer_init(int width, int height, const char *title, unsigned int target_fps) {
    (void)width;
    (void)height;
    (void)title;
    memset(framebuffer0, 0, sizeof(framebuffer0));
    lcd_init();
    set_target_fps(target_fps);
}

bool yr_game_should_close() {
    return 0;
}

void yr_begin_drawing() {
    int64_t now = esp_timer_get_time();
    cached_frame_time = (last_frame_start_us == 0)
                            ? 0.0f
                            : (float)(now - last_frame_start_us) / 1000000.0f;
    last_frame_start_us = now;
    frame_start_time_us = now;
}

void yr_render_screen() {
    lcd_present();
    // Block until every transfer of the frame is done, so the next frame's
    // drawing and conversion don't race the SPI read of this one.
    for (int i = 0; i < LCD_TRANSFERS; i++) {
        xSemaphoreTake(lcd_trans_done_sem, portMAX_DELAY);
    }
}

void yr_end_drawing() {
    if (target_fps > 0) {
        int64_t target_end_us = frame_start_time_us + target_frame_time_us;
        int64_t sleep_time_us = target_end_us - esp_timer_get_time();

        if (sleep_time_us > 0) {
            // Block-sleep the bulk (yields the CPU so the idle task feeds the
            // watchdog), then spin the sub-tick remainder so the frame period
            // stays tight and the FPS reads as stable rather than jittery.
            int64_t ms = sleep_time_us / 1000;
            if (ms > 1) vTaskDelay(pdMS_TO_TICKS(ms - 1));
            while (esp_timer_get_time() < target_end_us) { /* busy-wait */ }
        }
    }
}

float yr_get_frame_time() {
    return cached_frame_time;
}

float yr_get_time() {
    return esp_timer_get_time() / 1000000.0f; // Return time in seconds
}

float yr_get_fps() {
    if (cached_frame_time <= 0.0f) return 0.0f;
    return 1.0f / cached_frame_time;
}
