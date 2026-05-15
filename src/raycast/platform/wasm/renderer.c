#include "renderer.h"
#include <stdint.h>
#include <stddef.h>

extern void *malloc(size_t);
extern void *memset(void *, int, size_t);

__attribute__((import_module("env"), import_name("js_get_time")))
extern float js_get_time(void);

static pixel_t *framebuffer;
static int fb_width, fb_height;
static float last_time;
static float frame_time;

void renderer_init(int width, int height, const char* title, unsigned int target_fps) {
    (void)title;
    (void)target_fps;
    fb_width = width;
    fb_height = height;
    framebuffer = malloc(width * height * sizeof(pixel_t));
    memset(framebuffer, 0, width * height * sizeof(pixel_t));
    last_time = js_get_time();
}

bool game_should_close() {
    return false;
}

void begin_drawing() {
    float now = js_get_time();
    frame_time = now - last_time;
    last_time = now;
}

void render_screen() {}

void draw_rectangle(int x, int y, int w, int h, pixel_t c) {
    if (w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > fb_width)  w = fb_width - x;
    if (y + h > fb_height) h = fb_height - y;
    if (w <= 0 || h <= 0) return;

    for (int row = 0; row < h; row++) {
        int offset = (y + row) * fb_width + x;
        for (int col = 0; col < w; col++) {
            framebuffer[offset + col] = c;
        }
    }
}

void end_drawing() {
    // JS reads framebuffer directly from WASM memory
}

float get_frame_time() {
    return frame_time;
}

// Exports for JS
__attribute__((export_name("get_framebuffer")))
pixel_t *get_framebuffer(void) { return framebuffer; }

__attribute__((export_name("get_fb_width")))
int get_fb_width(void) { return fb_width; }

__attribute__((export_name("get_fb_height")))
int get_fb_height(void) { return fb_height; }

float get_fps() {
    if (frame_time <= 0.0f) return 0.0f;
    return 1.0f / frame_time;
}

float get_time() {
    return js_get_time(); // Return time in seconds
}