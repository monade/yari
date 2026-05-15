// Stub stdlib.h for WASM freestanding build
#ifndef _WASM_STDLIB_H
#define _WASM_STDLIB_H

#include <stddef.h>

void *malloc(size_t);
void *realloc(void *, size_t);
void free(void *);
int abs(int);
void qsort(void *, size_t, size_t, int (*)(const void *, const void *));

#endif
