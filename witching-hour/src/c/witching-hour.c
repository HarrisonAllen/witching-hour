#include <pebble.h>
#include "structs.h"
#include "defines.h"
#include "helpers.h"
#include "animations.h"

// Defines
#define SETTINGS_KEY 1

// Window
static Window *s_main_window;
// Layers
static TextLayer *s_time_layer, *s_date_layer;
static BitmapLayer *s_stars_layer, *s_moon_bm_layer, *s_cloud_layer, *s_weather_layer, *s_broom_layer, 
        *s_body_layer, *s_witch_layer, *s_cat_layer, *s_umbrella_layer;
static Layer *s_moon_layer;
// Resources
static GFont s_time_font, s_date_font;
static GBitmap *s_stars_bitmap, *s_moon_bitmap, *s_cloud_bitmap, *s_weather_bitmap, *s_broom_bitmap, 
        *s_body_bitmap, *s_witch_bitmap, *s_cat_bitmap, *s_umbrella_bitmap;
// Globals
static ClaySettings settings;
static bool weather_stale = true;
static int weather_stale_counter;
static AppTimer *anim_timer;
static FlyState fly_state;
static int16_t fly_offset_x;
static int fly_tick;
static FloatState float_state;
static int16_t float_offset_y;
static int float_tick;
static int float_cycle;
static FlyState weather_state;
static int16_t cloud_offset_y;
static int16_t weather_offset_y;
static int weather_tick;
static bool queue_screen_refresh = false;
int new_moon_frac = -1;
bool new_moon_waning;

static void temp_update_moon(time_t temp) {
  if (((temp / 20) % 2) == 1) {
    settings.MOON_FRACILLUM = 100-(temp % 20) * 5;
    settings.MOON_WANING = true;
  } else {
    settings.MOON_FRACILLUM = (temp % 20) * 5;
    settings.MOON_WANING = false;
  }
  layer_mark_dirty(s_moon_layer);
}

static void update_time() {
  static char s_time_buffer[8];
  static char s_date_buffer[12];

  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  if (settings.AmericanDate) {
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a %b %d", tick_time);
  } else {
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a %d %b", tick_time);
  }

  // temp_update_moon(temp);
  // snprintf(s_time_buffer, sizeof(s_time_buffer), "%d:%d", settings.MOON_FRACILLUM, settings.MOON_WANING ? 1 : 0);

  text_layer_set_text(s_time_layer, s_time_buffer);
  text_layer_set_text(s_date_layer, s_date_buffer);
}

// update the batter display layer
static void battery_callback(BatteryChargeState state) {
  uint32_t new_cat_resource = RESOURCE_ID_IMAGE_CAT_STANDING;

  if (s_cat_bitmap != NULL) {
    gbitmap_destroy(s_cat_bitmap);
    s_cat_bitmap = NULL;
  }
  if (state.is_charging) {
    new_cat_resource = RESOURCE_ID_IMAGE_CAT_STRETCHING;
  } else {
    if (state.charge_percent >= 80) {
      new_cat_resource = RESOURCE_ID_IMAGE_CAT_STANDING;
    } else if (state.charge_percent >= 50) {
      new_cat_resource = RESOURCE_ID_IMAGE_CAT_SITTING;
    } else if (state.charge_percent >= 20) {
      new_cat_resource = RESOURCE_ID_IMAGE_CAT_LOAFING;
    } else {
      new_cat_resource = RESOURCE_ID_IMAGE_CAT_SLEEPING;
    }
  }
  s_cat_bitmap = gbitmap_create_with_resource(new_cat_resource);
  bitmap_layer_set_bitmap(s_cat_layer, s_cat_bitmap);
}

static void bluetooth_callback(bool connected) {
  layer_set_hidden(bitmap_layer_get_layer(s_body_layer), !connected);

  if (!connected) {
    if (settings.VibrateOnDisc) {
      vibes_double_pulse();
    }
  }
}

static bool needs_umbrella(Weather conditions) {
  return conditions == RAINY
         || conditions == SNOWY
         || conditions == STORMY;
}

static uint32_t get_body_resource(Weather conditions) {
  return needs_umbrella(conditions) ? RESOURCE_ID_IMAGE_WITCH_BODY_UMBRELLA : RESOURCE_ID_IMAGE_WITCH_BODY_BASE;
}

static uint32_t get_witch_resource(Weather conditions, int temperature) {
  uint32_t new_witch_resource;
  uint32_t *witches = needs_umbrella(conditions) ? RAINY_WITCHES : SUNNY_WITCHES;
  if (temperature >= settings.Temperature4) {
    new_witch_resource = witches[4];
  } else if (temperature >= settings.Temperature3) {
    new_witch_resource = witches[3];
  } else if (temperature >= settings.Temperature2) {
    new_witch_resource = witches[2];
  } else if (temperature >= settings.Temperature1) {
    new_witch_resource = witches[1];
  } else {
    new_witch_resource = witches[0];
  }
  return new_witch_resource;
}

static uint32_t get_weather_resource(Weather conditions) {
  uint32_t new_weather_resource = 0;
  if (conditions == RAINY) {
    new_weather_resource = RESOURCE_ID_IMAGE_RAIN;
  } else if (conditions == SNOWY) {
    new_weather_resource = RESOURCE_ID_IMAGE_SNOW;
  } else if (conditions == STORMY) {
    new_weather_resource = RESOURCE_ID_IMAGE_STORM;
  }
  return new_weather_resource;
}

static uint32_t get_cloud_resource(Weather conditions) {
  uint32_t new_cloud_resource = 0;
  if (conditions == PARTLYCLOUDY) {
    new_cloud_resource = RESOURCE_ID_IMAGE_CLOUDS_PARTLY;
  } else {
    new_cloud_resource = RESOURCE_ID_IMAGE_CLOUDS_FULL;
  }
  return new_cloud_resource;
}

static void update_weather() {
  uint32_t new_cloud_resource = RESOURCE_ID_IMAGE_CLOUDS_PARTLY;

  layer_set_hidden(bitmap_layer_get_layer(s_umbrella_layer), !needs_umbrella(settings.CONDITIONS));

  // Conditions (body)
  if (s_body_bitmap != NULL) {
    gbitmap_destroy(s_body_bitmap);
    s_body_bitmap = NULL;
  }
  s_body_bitmap = gbitmap_create_with_resource(get_body_resource(settings.CONDITIONS));
  bitmap_layer_set_bitmap(s_body_layer, s_body_bitmap);
  
  // Temperature (witch)
  if (s_witch_bitmap != NULL) {
    gbitmap_destroy(s_witch_bitmap);
    s_witch_bitmap = NULL;
  }
  s_witch_bitmap = gbitmap_create_with_resource(get_witch_resource(settings.CONDITIONS, settings.TEMPERATURE));
  bitmap_layer_set_bitmap(s_witch_layer, s_witch_bitmap);

  // Conditions (background)
  if (s_weather_bitmap != NULL) {
    gbitmap_destroy(s_weather_bitmap);
    s_weather_bitmap = NULL;
  }
  if (needs_umbrella(settings.CONDITIONS)) {
    s_weather_bitmap = gbitmap_create_with_resource(get_weather_resource(settings.CONDITIONS));
    bitmap_layer_set_bitmap(s_weather_layer, s_weather_bitmap);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(s_weather_layer), true);
  }

  // Conditions (cloud)
  if (s_cloud_bitmap != NULL) {
    gbitmap_destroy(s_cloud_bitmap);
    s_cloud_bitmap = NULL;
  }
  if (settings.CONDITIONS != SUNNY) {
    s_cloud_bitmap = gbitmap_create_with_resource(get_cloud_resource(settings.CONDITIONS));
    bitmap_layer_set_bitmap(s_cloud_layer, s_cloud_bitmap);
  } else {
    layer_set_hidden(bitmap_layer_get_layer(s_cloud_layer), true);
  }
}

static bool is_in_ellipse(GRect ellipse_bounds, GPoint point, bool include_edge) {
  GPoint center = GPoint(ellipse_bounds.origin.x + ellipse_bounds.size.w / 2, ellipse_bounds.origin.y + ellipse_bounds.size.h / 2);
  float calculated = (squared(point.x - center.x) / squared(ellipse_bounds.size.w / 2))
                     + (squared(point.y - center.y) / squared(ellipse_bounds.size.h / 2));
  if (include_edge) {
    return calculated <= 1;
  } else {
    return calculated < 1;
  }
}

static void draw_ellipse(GContext *ctx, GRect ellipse_bounds, bool include_edge) {
  int x, y;
  for (x = ellipse_bounds.origin.x; x < ellipse_bounds.origin.x + ellipse_bounds.size.w; x++) {
    for (y = ellipse_bounds.origin.y; y < ellipse_bounds.origin.y + ellipse_bounds.size.h; y++) {
      if (is_in_ellipse(ellipse_bounds, GPoint(x, y), include_edge)) {
        graphics_draw_pixel(ctx, GPoint(x, y));
      }
    }
  }
}

static void moon_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_stroke_color(ctx, GColorWhite);
  draw_ellipse(ctx, bounds, false);
  graphics_context_set_fill_color(ctx, GColorBlack);
  if (settings.MOON_WANING) {
    graphics_fill_rect(ctx, GRect(bounds.origin.x + bounds.size.w / 2, bounds.origin.y, bounds.size.w / 2, bounds.size.h), 0, GCornerNone);
  } else {
    graphics_fill_rect(ctx, GRect(bounds.origin.x, bounds.origin.y, bounds.size.w / 2, bounds.size.h), 0, GCornerNone);
  }

  GPoint center = GPoint(bounds.origin.x + (bounds.size.w / 2), bounds.origin.y + (bounds.size.h / 2));
  int fracillum_width;
  if (settings.MOON_FRACILLUM > 50) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    fracillum_width = ((settings.MOON_FRACILLUM - 50) / 50.0) * (bounds.size.w / 2 + 3);
  } else {
    graphics_context_set_stroke_color(ctx, GColorBlack);
    fracillum_width = (1 - (settings.MOON_FRACILLUM / 50.0)) * (bounds.size.w / 2 + 3);
  }
  if (fracillum_width > FRACILLUM_MAX_WIDTH) {
    fracillum_width = bounds.size.w / 2;
  }
  GRect fracillum_bounds = GRect(center.x - fracillum_width, bounds.origin.y, fracillum_width * 2, bounds.size.h);
  
  draw_ellipse(ctx, fracillum_bounds, false);
}

static void update_moon() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  layer_set_hidden(bitmap_layer_get_layer(s_moon_bm_layer), !(tick_time->tm_mon == 9 && tick_time->tm_mday == 31));
  layer_set_hidden(s_moon_layer, (tick_time->tm_mon == 9 && tick_time->tm_mday == 31));
  if (new_moon_frac > -1) {
    settings.MOON_FRACILLUM = new_moon_frac;
    settings.MOON_WANING = new_moon_waning;
  }
  layer_mark_dirty(s_moon_layer);
}

static void set_witch_group_member_position(BitmapLayer *layer, int base_x, int base_y) {
  GRect og_bounds, new_bounds;
  og_bounds = layer_get_bounds(bitmap_layer_get_layer(layer));
  new_bounds = GRect(base_x + X_OFFSET + fly_offset_x, base_y + Y_OFFSET + float_offset_y, og_bounds.size.w, og_bounds.size.h);
  layer_set_frame(bitmap_layer_get_layer(layer), new_bounds);
}

static void set_witch_group_position() {
  set_witch_group_member_position(s_broom_layer, BROOM_X, BROOM_Y);
  set_witch_group_member_position(s_body_layer, BODY_X, BODY_Y);
  set_witch_group_member_position(s_witch_layer, WITCH_X, WITCH_Y);
  set_witch_group_member_position(s_cat_layer, CAT_X, CAT_Y);
  set_witch_group_member_position(s_umbrella_layer, UMBRELLA_X, UMBRELLA_Y);
}

static void set_weather_group_position() {
  GRect og_bounds, new_bounds;
  // cloud
  og_bounds = layer_get_bounds(bitmap_layer_get_layer(s_cloud_layer));
  new_bounds = GRect(CLOUDS_X + X_OFFSET, CLOUDS_Y + Y_OFFSET + cloud_offset_y, og_bounds.size.w, og_bounds.size.h);
  layer_set_frame(bitmap_layer_get_layer(s_cloud_layer), new_bounds);
  // moon
  og_bounds = layer_get_bounds(bitmap_layer_get_layer(s_moon_bm_layer));
  new_bounds = GRect(MOON_X + X_OFFSET, MOON_Y + Y_OFFSET + cloud_offset_y, og_bounds.size.w, og_bounds.size.h);
  layer_set_frame(bitmap_layer_get_layer(s_moon_bm_layer), new_bounds);
  layer_set_frame(s_moon_layer, new_bounds);
  // weather
  og_bounds = layer_get_bounds(bitmap_layer_get_layer(s_weather_layer));
  new_bounds = GRect(WEATHER_X + X_OFFSET, WEATHER_Y + Y_OFFSET + weather_offset_y, og_bounds.size.w, og_bounds.size.h);
  layer_set_frame(bitmap_layer_get_layer(s_weather_layer), new_bounds);
}

static bool is_witch_flying() {
  return (fly_state != ON_SCREEN && fly_state != OFF_SCREEN);
}

static bool is_witch_floating() {
  return (float_state != IDLE);
}

static bool is_witch_animating() {
  return is_witch_flying() || is_witch_floating();
}

static bool is_weather_animating() {
  return (weather_state != ON_SCREEN && weather_state != OFF_SCREEN);
}

static void animation_step(void *context);

static void start_witch_animation() {
  if (is_witch_flying()) {
    queue_screen_refresh = true;
  } else {
    // setup fly
    fly_tick = 0;
    if (fly_state == ON_SCREEN) {
      fly_state = FLYING_OUT;
      fly_offset_x = FLY_OUT_CURVE_1[fly_tick];
    } else {
      fly_state = FLYING_IN;
      fly_offset_x = FLY_IN_CURVE_1[fly_tick];
    }
    // setup float
    if (!is_witch_floating()) {
      float_tick = 0;
      float_offset_y = FLOAT_CURVE_2[float_tick / FLOAT_SLOWER];
      float_state = FLOATING;
    }
    float_cycle = 0;
    set_witch_group_position();
    if (anim_timer == NULL) anim_timer = app_timer_register(TICK_DURATION, animation_step, NULL);
  }
}

static void start_weather_animation() {
  if (!is_weather_animating()) {
    weather_tick = 0;
    if (weather_state == ON_SCREEN) {
      weather_state = FLYING_OUT;
      weather_offset_y = WEATHER_OUT_CURVE[weather_tick];
      cloud_offset_y = CLOUD_OUT_CURVE[weather_tick];
    } else {
      weather_state = FLYING_IN;
      weather_offset_y = WEATHER_IN_CURVE[weather_tick];
      cloud_offset_y = CLOUD_IN_CURVE[weather_tick];
    }
    set_weather_group_position();
    if (anim_timer == NULL) anim_timer = app_timer_register(TICK_DURATION, animation_step, NULL);
  }
}

static void request_weather();

static void animation_step(void *context) {
  // Handle weather
  if (weather_state == FLYING_OUT) {
    weather_offset_y = WEATHER_OUT_CURVE[weather_tick];
    cloud_offset_y = CLOUD_OUT_CURVE[weather_tick];
    weather_tick += 1;
    if (weather_tick >= WEATHER_TICKS) {
      weather_state = OFF_SCREEN;
    }
    set_weather_group_position();
  } else if (weather_state == FLYING_IN) {
    weather_offset_y = WEATHER_IN_CURVE[weather_tick];
    cloud_offset_y = CLOUD_IN_CURVE[weather_tick];
    weather_tick += 1;
    if (weather_tick >= WEATHER_TICKS) {
      weather_state = ON_SCREEN;
    }
    set_weather_group_position();
  } else {
    // Handle fly
    if (fly_state == FLYING_OUT) {
      fly_offset_x = FLY_OUT_CURVE_1[fly_tick];
      fly_tick += 1;
      if (fly_tick >= FLY_TICKS) {
        fly_state = OFF_SCREEN;
      }
    } else if (fly_state == FLYING_IN) {
      fly_offset_x = FLY_IN_CURVE_1[fly_tick];
      fly_tick += 1;
      if (fly_tick >= FLY_TICKS) {
        fly_state = ON_SCREEN;
      }
    }

    // Handle float
    if (float_state == FLOATING) {
      if (fly_state == OFF_SCREEN) {
        float_state = IDLE;
        float_offset_y = 0;
      } else {
        float_offset_y = FLOAT_CURVE_2[float_tick / FLOAT_SLOWER];
        float_tick += 1;
        if (float_tick >= FLOAT_TICKS) {
          float_cycle++;
          if (float_cycle >= FLOAT_CYCLES) {
            float_state = IDLE;
            float_offset_y = 0;
          } else {
            float_tick = 0;
          }
        }
      }
    }

    set_witch_group_position();
  }
  
  if (is_witch_flying() || is_weather_animating()) {
    anim_timer = app_timer_register(TICK_DURATION, animation_step, NULL);
  } else {
    if (weather_state == ON_SCREEN) {
      if (fly_state == ON_SCREEN) {
        if (weather_stale) {
          request_weather();
        }
        if (queue_screen_refresh) {
          anim_timer = NULL;
          start_witch_animation();
        } else if (is_witch_floating()) {
          anim_timer = app_timer_register(TICK_DURATION, animation_step, NULL);
        } else {
          anim_timer = NULL;
        }
      } else {
        anim_timer = NULL;
        if (queue_screen_refresh) {
          start_weather_animation();
        } else {
          start_witch_animation();
        }
      }
    } else {
      anim_timer = NULL;
      update_weather();
      update_moon();
      start_weather_animation();
      queue_screen_refresh = false;
    }
  }
}

static void default_settings() {  
  settings.TEMPERATURE = 80;              // placeholder temperature
  settings.CONDITIONS = RAINY;           // placeholder weather
  settings.MOON_FRACILLUM = 30;          // placeholder fracillum
  settings.MOON_WANING = true;

  settings.UseCurrentLocation = true;     // use GPS for weather
  settings.WeatherCheckRate = 15;         // check every 15 mins
  strcpy(settings.Latitude, "42.36");     // MIT latitude
  strcpy(settings.Longitude, "-71.1");     // MIT longitude
  settings.AmericanDate = true;           // Fri Oct 31 by default
  settings.VibrateOnDisc = true;          // vibrate by default
  
  settings.TemperatureMetric = false;        // Celsius or Fahrenheit?
  settings.Temperature1 = 40;                // Cold temperature
  settings.Temperature2 = 60;                // Chilly temperature
  settings.Temperature3 = 75;                // Warm temperature
  settings.Temperature4 = 90;                // Hot temperature
}

static void load_settings() {
  default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void request_weather() {
  weather_stale = false;
  weather_stale_counter = 0;
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);

  if (result == APP_MSG_OK) {
    // tell the app whether to use current location, celsius, and also the lat and lon
    dict_write_uint8(iter, MESSAGE_KEY_UseCurrentLocation, settings.UseCurrentLocation);
    dict_write_uint8(iter, MESSAGE_KEY_TemperatureMetric, settings.TemperatureMetric);
    dict_write_cstring(iter, MESSAGE_KEY_Latitude, settings.Latitude);
    dict_write_cstring(iter, MESSAGE_KEY_Longitude, settings.Longitude);

    // Send the message
    result = app_message_outbox_send();
  }
}

// Received data! Either for weather, moon, or settings
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  bool trigger_animation = false;
  uint32_t old_weather_resource = get_weather_resource(settings.CONDITIONS);
  uint32_t old_cloud_resource = get_cloud_resource(settings.CONDITIONS);
  uint32_t old_body_resource = get_body_resource(settings.CONDITIONS);
  uint32_t old_witch_resource = get_witch_resource(settings.CONDITIONS, settings.TEMPERATURE);

  // Current temperature and weather conditions
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);
  if (temp_tuple && conditions_tuple) {
    int new_temp = (int)temp_tuple->value->int32;
    Weather new_weather = (Weather)conditions_tuple->value->int32;
    settings.TEMPERATURE = new_temp;
    settings.CONDITIONS = new_weather;
    weather_stale = false;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature: %d - Conditions: %d", settings.TEMPERATURE, settings.CONDITIONS);
  }

  // Current moon conditions
  Tuple *frac_tuple = dict_find(iterator, MESSAGE_KEY_MOON_FRACILLUM);
  Tuple *waning_tuple = dict_find(iterator, MESSAGE_KEY_MOON_WANING);
  if (frac_tuple && waning_tuple) {
    new_moon_frac = (int)frac_tuple->value->int32;
    new_moon_waning = waning_tuple->value->int32 == 1;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Fracillum: %d - Waning: %d", new_moon_frac, new_moon_waning ? 1 : 0);
  }

  // American date format?
  Tuple *american_date_t = dict_find(iterator, MESSAGE_KEY_AmericanDate);
  if(american_date_t) {
    settings.AmericanDate = american_date_t->value->int32 == 1;
  }
  // Use current location
  Tuple *use_current_location_t = dict_find(iterator, MESSAGE_KEY_UseCurrentLocation);
  if(use_current_location_t) {
    settings.UseCurrentLocation = use_current_location_t->value->int32 == 1;
  }

  // Weather update rate
  Tuple *weather_check_rate_t = dict_find(iterator, MESSAGE_KEY_WeatherCheckRate);
  if(weather_check_rate_t) {
    settings.WeatherCheckRate = weather_check_rate_t->value->int32;
  }

  // Manual Latitude
  Tuple *latitude_t = dict_find(iterator, MESSAGE_KEY_Latitude);
  if(latitude_t) {
    strcpy(settings.Latitude,latitude_t->value->cstring);
  }

  // Manual Longitude
  Tuple *longitude_t = dict_find(iterator, MESSAGE_KEY_Longitude);
  if(longitude_t) {
    strcpy(settings.Longitude,longitude_t->value->cstring);
  }

  // Celsius or Fahrenheit?
  Tuple *temperature_metric_t = dict_find(iterator, MESSAGE_KEY_TemperatureMetric);
  if(temperature_metric_t) {
    settings.TemperatureMetric = temperature_metric_t->value->int32 == 1;
  }

  // Cold temperature
  Tuple *temperature_1_t = dict_find(iterator, MESSAGE_KEY_Temperature1);
  if(temperature_1_t) {
    settings.Temperature1 = atoi(temperature_1_t->value->cstring);
  }

  // Comfortable temperature
  Tuple *temperature_2_t = dict_find(iterator, MESSAGE_KEY_Temperature2);
  if(temperature_2_t) {
    settings.Temperature2 = atoi(temperature_2_t->value->cstring);
  }

  // Hot temperature
  Tuple *temperature_3_t = dict_find(iterator, MESSAGE_KEY_Temperature3);
  if(temperature_3_t) {
    settings.Temperature3 = atoi(temperature_3_t->value->cstring);
  }

  // Hottest temperature
  Tuple *temperature_4_t = dict_find(iterator, MESSAGE_KEY_Temperature4);
  if(temperature_4_t) {
    settings.Temperature4 = atoi(temperature_4_t->value->cstring);
  }

  // Vibrate on disconnect
  Tuple *vibrate_on_disc_t = dict_find(iterator, MESSAGE_KEY_VibrateOnDisc);
  if(vibrate_on_disc_t) {
    settings.VibrateOnDisc = vibrate_on_disc_t->value->int32 == 1;
  }


  update_time();
  trigger_animation = old_weather_resource != get_weather_resource(settings.CONDITIONS)
                      || old_cloud_resource != get_cloud_resource(settings.CONDITIONS)
                      || old_body_resource != get_body_resource(settings.CONDITIONS)
                      || old_witch_resource != get_witch_resource(settings.CONDITIONS, settings.TEMPERATURE)
                      || settings.MOON_FRACILLUM != new_moon_frac
                      || settings.MOON_WANING != new_moon_waning;
  if (trigger_animation) {
    queue_screen_refresh = true;
    start_witch_animation();
  }

  save_settings(); // save the new settings! Current weather included
}

// Message failed to receive
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

// Message failed to send
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

// Message sent successfully
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changes) {
  update_time();
  weather_stale_counter++;
  if (weather_stale_counter > settings.WeatherCheckRate) {
    if (is_witch_animating() || is_weather_animating()) {
      weather_stale = true;
    } else {
      request_weather();
    }
  }
}

// setup the display
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // stars: 0, 0
  s_stars_layer = bitmap_layer_create(GRect(STARS_X + X_OFFSET, STARS_Y + Y_OFFSET, 180, 57));
  s_stars_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STARS);
  bitmap_layer_set_bitmap(s_stars_layer, s_stars_bitmap);
  
  // weather: 0, 48
  s_weather_layer = bitmap_layer_create(GRect(WEATHER_X + X_OFFSET, WEATHER_Y + Y_OFFSET, 180, 132));
  s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN);
  bitmap_layer_set_bitmap(s_weather_layer, s_weather_bitmap);
  
  // pumpkin moon: 73, 19
  s_moon_bm_layer = bitmap_layer_create(GRect(MOON_X + X_OFFSET, MOON_Y + Y_OFFSET, 34, 33));
  s_moon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_PUMPKIN_MOON);
  bitmap_layer_set_bitmap(s_moon_bm_layer, s_moon_bitmap);
  layer_set_hidden(bitmap_layer_get_layer(s_moon_bm_layer), true);

  // real moon: 73, 19
  bitmap_layer_set_compositing_mode(s_moon_bm_layer, GCompOpSet);
  s_moon_layer = layer_create(GRect(MOON_X + X_OFFSET, MOON_Y + Y_OFFSET, MOON_SIZE, MOON_SIZE));
  layer_set_update_proc(s_moon_layer, moon_update_proc);

  // clouds: 0, 0
  s_cloud_layer = bitmap_layer_create(GRect(CLOUDS_X + X_OFFSET, CLOUDS_Y + Y_OFFSET, 180, 57));
  s_cloud_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDS_FULL);
  bitmap_layer_set_bitmap(s_cloud_layer, s_cloud_bitmap);
  bitmap_layer_set_compositing_mode(s_cloud_layer, GCompOpSet);

  // time: x, 108 ish
  s_time_layer = text_layer_create(GRect(TIME_X, TIME_Y + Y_OFFSET, bounds.size.w, 38));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_34));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // date: x, 144 ish
  s_date_layer = text_layer_create(GRect(DATE_X, DATE_Y + Y_OFFSET, bounds.size.w, 24));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_20));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // broom: 50, 66
  s_broom_layer = bitmap_layer_create(GRect(FLY_START_X + BROOM_X + X_OFFSET, BROOM_Y + Y_OFFSET, 81, 52));
  s_broom_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BROOM);
  bitmap_layer_set_bitmap(s_broom_layer, s_broom_bitmap);
  bitmap_layer_set_compositing_mode(s_broom_layer, GCompOpSet);

  // Body: 67, 70
  s_body_layer = bitmap_layer_create(GRect(FLY_START_X + BODY_X + X_OFFSET, BODY_Y + Y_OFFSET, 37, 47));
  s_body_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WITCH_BODY_UMBRELLA);
  bitmap_layer_set_bitmap(s_body_layer, s_body_bitmap);
  bitmap_layer_set_compositing_mode(s_body_layer, GCompOpSet);

  // Witch: 67, 70
  s_witch_layer = bitmap_layer_create(GRect(FLY_START_X + WITCH_X + X_OFFSET, WITCH_Y + Y_OFFSET, 37, 47));
  s_witch_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WITCH_WARM_UMBRELLA);
  bitmap_layer_set_bitmap(s_witch_layer, s_witch_bitmap);
  bitmap_layer_set_compositing_mode(s_witch_layer, GCompOpSet);
  
  // Cat: 102, 89
  s_cat_layer = bitmap_layer_create(GRect(FLY_START_X + CAT_X + X_OFFSET, CAT_Y + Y_OFFSET, 22, 14));
  s_cat_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CAT_LOAFING);
  bitmap_layer_set_bitmap(s_cat_layer, s_cat_bitmap);
  bitmap_layer_set_compositing_mode(s_cat_layer, GCompOpSet);

  // umbrella: 74, 63
  s_umbrella_layer = bitmap_layer_create(GRect(FLY_START_X + UMBRELLA_X + X_OFFSET, UMBRELLA_Y + Y_OFFSET, 52, 35));
  s_umbrella_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UMBRELLA);
  bitmap_layer_set_bitmap(s_umbrella_layer, s_umbrella_bitmap);
  bitmap_layer_set_compositing_mode(s_umbrella_layer, GCompOpSet);
  

  layer_add_child(window_layer, bitmap_layer_get_layer(s_stars_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_weather_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_moon_bm_layer));
  layer_add_child(window_layer, s_moon_layer);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_cloud_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_broom_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_body_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_witch_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_cat_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_umbrella_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

// unload everything!
static void main_window_unload(Window *window) {
  // unload regular layers
  if (s_moon_layer != NULL)
    layer_destroy(s_moon_layer);

  // unload text layers
  if (s_time_layer != NULL)
    text_layer_destroy(s_time_layer);
  if (s_date_layer != NULL)
    text_layer_destroy(s_date_layer);

  // unload custom fonts
  if (s_time_font != NULL)
    fonts_unload_custom_font(s_time_font);
  if (s_date_font != NULL)
    fonts_unload_custom_font(s_date_font);

  // unload bitmap layers
  if (s_stars_layer != NULL)
    bitmap_layer_destroy(s_stars_layer);
  if (s_moon_bm_layer != NULL)
    bitmap_layer_destroy(s_moon_bm_layer);
  if (s_cloud_layer != NULL)
    bitmap_layer_destroy(s_cloud_layer);
  if (s_weather_layer != NULL)
    bitmap_layer_destroy(s_weather_layer);
  if (s_broom_layer != NULL)
    bitmap_layer_destroy(s_broom_layer);
  if (s_body_layer != NULL)
    bitmap_layer_destroy(s_body_layer);
  if (s_witch_layer != NULL)
    bitmap_layer_destroy(s_witch_layer);
  if (s_cat_layer != NULL)
    bitmap_layer_destroy(s_cat_layer);
  if (s_umbrella_layer != NULL)
    bitmap_layer_destroy(s_umbrella_layer);

  // unload gbitmaps
  if (s_stars_bitmap != NULL)
    gbitmap_destroy(s_stars_bitmap);
  if (s_moon_bitmap != NULL)
    gbitmap_destroy(s_moon_bitmap);
  if (s_cloud_bitmap != NULL)
    gbitmap_destroy(s_cloud_bitmap);
  if (s_weather_bitmap != NULL)
    gbitmap_destroy(s_weather_bitmap);
  if (s_broom_bitmap != NULL)
    gbitmap_destroy(s_broom_bitmap);
  if (s_witch_bitmap != NULL)
    gbitmap_destroy(s_body_bitmap);
  if (s_cat_bitmap != NULL)
    gbitmap_destroy(s_witch_bitmap);
  if (s_cat_bitmap != NULL)
    gbitmap_destroy(s_cat_bitmap);
  if (s_umbrella_bitmap != NULL)
    gbitmap_destroy(s_umbrella_bitmap);
}

// static void tick_handler(struct tm *tick_time, TimeUnits units_changes) {
//   update_time(); // display the time

//   // update the weather
//   s_weather_minutes_elapsed++;
//   if (s_weather_minutes_elapsed >= settings.WeatherCheckRate) { // time to check the weather
//     // get the weather
//     request_weather();

//     s_weather_minutes_elapsed = 0;
//   } 

//   // update date on first call, or at midnight (00:00)
//   if (!s_date_set || (tick_time->tm_min == 0 && tick_time->tm_hour == 0)) {
//     update_date(tick_time);
//     // if this is the last to load, then animate!
//     s_date_set = true;
//   }
// }

static void init() {
  load_settings();

  // setup window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_background_color(s_main_window, GColorBlack);

  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();
  update_weather();
  update_moon();

  start_weather_animation();

  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // Register callbacks for settings/weather updates
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  const int inbox_size = 1024; // maaaaybe overkill, but 128 isn't enough
  const int outbox_size = 1024;
  app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
