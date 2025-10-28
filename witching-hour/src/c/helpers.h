#include <pebble.h>
#pragma once

static int lerp(int a, int b, float c){
    return (int)(a * (1.0 - c) + b * c);
}

static float squared(float x) {
    return x*x;
}

static int f_to_c(float f) {
    return (f - 32) * 5.0 / 9.0;
}

static int c_to_f(float c) {
    return c * 9.0 / 5.0 + 32;
}