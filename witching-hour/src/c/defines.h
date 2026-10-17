#include <pebble.h>
#pragma once

#define X_OFFSET ((PBL_DISPLAY_WIDTH - 180) / 2)
#define Y_OFFSET ((PBL_DISPLAY_HEIGHT - 180) / 2)
#define LARGE_SCREEN_OFFSET (-40)
#define MOON_SIZE 35
#define FRACILLUM_MAX_WIDTH 16

#if defined(PBL_PLATFORM_GABBRO) || defined(PBL_PLATFORM_EMERY)
#define LARGE_SCREEN true
#else
#define LARGE_SCREEN false
#endif

// Positions and sizes
// Positions relative to PTR/Chalk origin
#define STARS_X (LARGE_SCREEN ? (0 + LARGE_SCREEN_OFFSET) : 0)
#define STARS_Y (LARGE_SCREEN ? (0 + LARGE_SCREEN_OFFSET) : 0)
#define STARS_W (LARGE_SCREEN ? 260 : 180)
#define STARS_H (LARGE_SCREEN ? 97 : 57)
#define WEATHER_X (LARGE_SCREEN ? (0 + LARGE_SCREEN_OFFSET) : 0)
#define WEATHER_Y (LARGE_SCREEN ? (82 + LARGE_SCREEN_OFFSET) : 48)
#define WEATHER_W (LARGE_SCREEN ? 260 : 180)
#define WEATHER_H (LARGE_SCREEN ? 178 : 132)
#define MOON_X 73
#define MOON_Y 19
#define MOON_W 34
#define MOON_H 33
#define CLOUDS_X (LARGE_SCREEN ? (0 + LARGE_SCREEN_OFFSET) : 0)
#define CLOUDS_Y (LARGE_SCREEN ? (0 + LARGE_SCREEN_OFFSET) : 0)
#define CLOUDS_W (LARGE_SCREEN ? 260 : 180)
#define CLOUDS_H (LARGE_SCREEN ? 96 : 57)
#define TIME_X 0
#define TIME_Y (LARGE_SCREEN ? 108 : 108)
#define TIME_H (LARGE_SCREEN ? 68 : 38)
#if LARGE_SCREEN
#define TIME_FONT RESOURCE_ID_FONT_TIME_48
#else
#define TIME_FONT RESOURCE_ID_FONT_TIME_34
#endif
#define DATE_X 0
#define DATE_Y (LARGE_SCREEN ? 160 : 144)
#define DATE_H (LARGE_SCREEN ? 48 : 24)
#if LARGE_SCREEN
#define DATE_FONT RESOURCE_ID_FONT_DATE_32
#else
#define DATE_FONT RESOURCE_ID_FONT_DATE_20
#endif
#define BROOM_X 50
#define BROOM_Y 66
#define BROOM_W 81
#define BROOM_H 52
#define BODY_X 67
#define BODY_Y 70
#define BODY_W 37
#define BODY_H 47
#define WITCH_X 67
#define WITCH_Y 70
#define WITCH_W 37
#define WITCH_H 47
#define CAT_X 102
#define CAT_Y 89
#define CAT_W 22
#define CAT_H 14
#define UMBRELLA_X 74
#define UMBRELLA_Y 63
#define UMBRELLA_W 52
#define UMBRELLA_H 35
