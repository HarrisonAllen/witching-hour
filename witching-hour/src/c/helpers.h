#include <pebble.h>
#pragma once

static int lerp(int a, int b, float c){
    return (int)(a * (1.0 - c) + b * c);
}

static float squared(float x) {
    return x*x;
}