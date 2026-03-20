// Minimal runtime for freestanding wasm32 build
// Provides: memory allocator, qsort, memset/memcpy, math imports from JS

#include <stddef.h>
#include <stdint.h>

// =================== Math imports from JS ===================
#define MATH_IMPORT __attribute__((import_module("env")))

MATH_IMPORT __attribute__((import_name("sin"))) double __imported_sin(double);
MATH_IMPORT __attribute__((import_name("cos"))) double __imported_cos(double);
MATH_IMPORT __attribute__((import_name("tan"))) double __imported_tan(double);
MATH_IMPORT __attribute__((import_name("sqrt"))) double __imported_sqrt(double);
MATH_IMPORT __attribute__((import_name("floor"))) double __imported_floor(double);
MATH_IMPORT __attribute__((import_name("fabs"))) double __imported_fabs(double);

float sinf(float x)   { return (float)__imported_sin(x); }
float cosf(float x)   { return (float)__imported_cos(x); }
float tanf(float x)   { return (float)__imported_tan(x); }
float sqrtf(float x)  { return (float)__imported_sqrt(x); }
float floorf(float x) { return (float)__imported_floor(x); }
double fabs(double x) { return __imported_fabs(x); }
float fabsf(float x)  { return (float)__imported_fabs(x); }
float fminf(float a, float b)  { return a < b ? a : b; }
float fmaxf(float a, float b)  { return a > b ? a : b; }

// =================== Memory operations ===================

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n; s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

// =================== Simple bump allocator ===================

extern unsigned char __heap_base;
static unsigned char *heap_ptr = 0;

void *malloc(size_t size) {
    if (!heap_ptr) heap_ptr = &__heap_base;
    // align to 8 bytes
    size = (size + 7) & ~7;
    void *p = heap_ptr;
    heap_ptr += size;
    return p;
}

void free(void *ptr) {
    (void)ptr; // bump allocator doesn't free
}

// =================== qsort ===================

static void swap_bytes(void *a, void *b, size_t size) {
    unsigned char *pa = a, *pb = b;
    for (size_t i = 0; i < size; i++) {
        unsigned char tmp = pa[i];
        pa[i] = pb[i];
        pb[i] = tmp;
    }
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *)) {
    // Simple insertion sort (good enough for small sprite arrays)
    unsigned char *arr = base;
    for (size_t i = 1; i < nmemb; i++) {
        size_t j = i;
        while (j > 0 && compar(arr + j * size, arr + (j - 1) * size) < 0) {
            swap_bytes(arr + j * size, arr + (j - 1) * size, size);
            j--;
        }
    }
}

// =================== abs ===================

int abs(int x) { return x < 0 ? -x : x; }
