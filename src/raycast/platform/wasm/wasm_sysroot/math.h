// Stub math.h for WASM freestanding build
// Actual implementations are in wasm_runtime.c (imported from JS)
#ifndef _WASM_MATH_H
#define _WASM_MATH_H

double sin(double);
double cos(double);
double tan(double);
double asin(double);
double acos(double);
double atan2(double, double);
double sqrt(double);
double floor(double);
double ceil(double);
double fabs(double);
double fmod(double, double);
double round(double);
double pow(double, double);

float sinf(float);
float cosf(float);
float tanf(float);
float asinf(float);
float acosf(float);
float atan2f(float, float);
float sqrtf(float);
float floorf(float);
float ceilf(float);
float fabsf(float);
float fmodf(float, float);
float roundf(float);
float fminf(float, float);
float fmaxf(float, float);

#endif
