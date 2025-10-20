#include <pebble.h>
#pragma once

// Overall
#define TICK_DURATION 100

// Flying
#define FLY_START_X -131
#define FLY_START_VEL 25
#define FLY_ACC 1
#define FLY_TICKS 11

// Floating
#define FLOAT_MAX_VEL 3
#define FLOAT_ACC 1
#define FLOAT_SLOWER 1
#define FLOAT_TICKS (21 * FLOAT_SLOWER)
#define FLOAT_CYCLES 2

static const int FLY_IN_CURVE_1[FLY_TICKS] = {-131, -106, -84, -64, -47, -33, -21, -12, -5, -1, 0};
static const int FLY_IN_CURVE_2[FLY_TICKS] = {-131, -130, -126, -119, -110, -98, -84, -67, -47, -25, 0};
static const int FLY_OUT_CURVE_1[FLY_TICKS] = {0, 1, 5, 12, 21, 33, 47, 64, 84, 106, 131};
static const int FLY_OUT_CURVE_2[FLY_TICKS] = {0, 25, 47, 67, 84, 98, 110, 119, 126, 130, 131};
static const int FLOAT_CURVE_5[FLOAT_TICKS] = {0, 1, 3, 4, 5, 5, 5, 4, 3, 2, 1, -1, -2, -3, -4, -5, -5, -5, -4, -3, -1};
static const int FLOAT_CURVE_4[FLOAT_TICKS] = {0, 1, 2, 3, 4, 4, 4, 3, 3, 2, 1, -1, -2, -3, -3, -4, -4, -4, -3, -2, -1};
static const int FLOAT_CURVE_3[FLOAT_TICKS] = {0, 1, 2, 2, 3, 3, 3, 3, 2, 1, 0, 0, -1, -2, -3, -3, -3, -3, -2, -2, -1};
static const int FLOAT_CURVE_2[FLOAT_TICKS] = {0, 1, 1, 2, 2, 2, 2, 2, 1, 1, 0, 0, -1, -1, -2, -2, -2, -2, -2, -1, -1};

// [1.0, 0.81, 0.64, 0.49, 0.36, 0.25, 0.16, 0.09, 0.04, 0.01, 0.0]
// [1.0, 0.99, 0.96, 0.91, 0.84, 0.75, 0.64, 0.51, 0.36, 0.19, 0.0]
// [0.0, 0.01, 0.04, 0.09, 0.16, 0.25, 0.36, 0.49, 0.64, 0.81, 1.0]
// [0.0, 0.19, 0.36, 0.51, 0.64, 0.75, 0.84, 0.91, 0.96, 0.99, 1.0]
// sin(2*pi*i/21)*5