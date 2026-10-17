#include <pebble.h>
#include "defines.h"
#pragma once

// Overall
#define TICK_DURATION 100

// Flying
#define FLY_START_X (LARGE_SCREEN ? -171 : -131)
#define FLY_START_VEL 25
#define FLY_ACC 1
#define FLY_TICKS 11

// Floating
#define FLOAT_MAX_VEL 3
#define FLOAT_ACC 1
#define FLOAT_SLOWER 1
#define FLOAT_TICKS (21 * FLOAT_SLOWER)
#define FLOAT_CYCLES 2

#define WEATHER_TICKS 11

#if LARGE_SCREEN
// Curves
// - Flying
static const int FLY_IN_CURVE_1[FLY_TICKS] = {-171, -139, -109, -84, -62, -43, -27, -15, -7, -2, 0};
static const int FLY_IN_CURVE_2[FLY_TICKS] = {-171, -169, -164, -156, -144, -128, -109, -87, -62, -32, 0};
static const int FLY_OUT_CURVE_1[FLY_TICKS] = {0, 2, 7, 15, 27, 43, 62, 84, 109, 139, 171};
static const int FLY_OUT_CURVE_2[FLY_TICKS] = {0, 32, 62, 87, 109, 128, 144, 156, 164, 169, 171};
// - Floating
static const int FLOAT_CURVE_5[FLOAT_TICKS] = {0, 1, 3, 4, 5, 5, 5, 4, 3, 2, 1, -1, -2, -3, -4, -5, -5, -5, -4, -3, -1};
static const int FLOAT_CURVE_4[FLOAT_TICKS] = {0, 1, 2, 3, 4, 4, 4, 3, 3, 2, 1, -1, -2, -3, -3, -4, -4, -4, -3, -2, -1};
static const int FLOAT_CURVE_3[FLOAT_TICKS] = {0, 1, 2, 2, 3, 3, 3, 3, 2, 1, 0, 0, -1, -2, -3, -3, -3, -3, -2, -2, -1};
static const int FLOAT_CURVE_2[FLOAT_TICKS] = {0, 1, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0, -1, -1, -2, -2, -2, -2, -2, -1, -1};
// - Clouds
static const int CLOUD_IN_CURVE[WEATHER_TICKS] = {-96, -78, -61, -47, -35, -24, -15, -9, -4, -1, 0};
static const int CLOUD_OUT_CURVE[WEATHER_TICKS] = {0, -1, -4, -9, -15, -24, -35, -47, -61, -78, -96};
// - Weather
static const int WEATHER_IN_CURVE[WEATHER_TICKS] = {260, 211, 166, 127, 94, 65, 42, 23, 10, 3, 0};
static const int WEATHER_OUT_CURVE[WEATHER_TICKS] = {0, 3, 10, 23, 42, 65, 94, 127, 166, 211, 260};
#else
// Curves
// - Flying
static const int FLY_IN_CURVE_1[FLY_TICKS] = {-131, -106, -84, -64, -47, -33, -21, -12, -5, -1, 0};
static const int FLY_IN_CURVE_2[FLY_TICKS] = {-131, -130, -126, -119, -110, -98, -84, -67, -47, -25, 0};
static const int FLY_OUT_CURVE_1[FLY_TICKS] = {0, 1, 5, 12, 21, 33, 47, 64, 84, 106, 131};
static const int FLY_OUT_CURVE_2[FLY_TICKS] = {0, 25, 47, 67, 84, 98, 110, 119, 126, 130, 131};
// - Floating
static const int FLOAT_CURVE_5[FLOAT_TICKS] = {0, 1, 3, 4, 5, 5, 5, 4, 3, 2, 1, -1, -2, -3, -4, -5, -5, -5, -4, -3, -1};
static const int FLOAT_CURVE_4[FLOAT_TICKS] = {0, 1, 2, 3, 4, 4, 4, 3, 3, 2, 1, -1, -2, -3, -3, -4, -4, -4, -3, -2, -1};
static const int FLOAT_CURVE_3[FLOAT_TICKS] = {0, 1, 2, 2, 3, 3, 3, 3, 2, 1, 0, 0, -1, -2, -3, -3, -3, -3, -2, -2, -1};
static const int FLOAT_CURVE_2[FLOAT_TICKS] = {0, 1, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0, -1, -1, -2, -2, -2, -2, -2, -1, -1};
// - Clouds
static const int CLOUD_IN_CURVE[WEATHER_TICKS] = {-57, -46, -36, -28, -21, -14, -9, -5, -2, -1, 0};
static const int CLOUD_OUT_CURVE[WEATHER_TICKS] = {0, -1, -2, -5, -9, -14, -21, -28, -36, -46, -57};
// - Weather
static const int WEATHER_IN_CURVE[WEATHER_TICKS] = {180, 146, 115, 88, 65, 45, 29, 16, 7, 2, 0};
static const int WEATHER_OUT_CURVE[WEATHER_TICKS] = {0, 2, 7, 16, 29, 45, 65, 88, 115, 146, 180};
#endif

// [1.0, 0.81, 0.64, 0.49, 0.36, 0.25, 0.16, 0.09, 0.04, 0.01, 0.0]
// [1.0, 0.99, 0.96, 0.91, 0.84, 0.75, 0.64, 0.51, 0.36, 0.19, 0.0]
// [0.0, 0.01, 0.04, 0.09, 0.16, 0.25, 0.36, 0.49, 0.64, 0.81, 1.0]
// [0.0, 0.19, 0.36, 0.51, 0.64, 0.75, 0.84, 0.91, 0.96, 0.99, 1.0]
// sin(2*pi*i/21)*5

/**
# Python script for this conversion
a = [1.0, 0.81, 0.64, 0.49, 0.36, 0.25, 0.16, 0.09, 0.04, 0.01, 0.0]
b = [1.0, 0.99, 0.96, 0.91, 0.84, 0.75, 0.64, 0.51, 0.36, 0.19, 0.0]
c = [0.0, 0.01, 0.04, 0.09, 0.16, 0.25, 0.36, 0.49, 0.64, 0.81, 1.0]
d = [0.0, 0.19, 0.36, 0.51, 0.64, 0.75, 0.84, 0.91, 0.96, 0.99, 1.0]
def conv(start, t):
    out = ["{"]
    for i in t:
        out.append(str(round(start * i)))
        out.append(", ")
    out[-1] = "}"
    out.append(";")
    return "".join(out)
*/