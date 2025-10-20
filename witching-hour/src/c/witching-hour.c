#include <pebble.h>
#include "structs.h"
#include "defines.h"
#include "helpers.h"

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
static int16_t fly_offset_x;
static int fly_start_x;
static int16_t float_offset_y;
static int float_start_y, float_end_y;
static int frames[FLOAT_COUNTS + 1];
static bool got_weather = false;

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

static void tick_handler(struct tm *tick_time, TimeUnits units_changes) {
  update_time();
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

static bool needs_umbrella() {
  return settings.CONDITIONS == RAINY
         || settings.CONDITIONS == SNOWY
         || settings.CONDITIONS == STORMY;
}

static void update_weather() {
  uint32_t new_body_resource = RESOURCE_ID_IMAGE_WITCH_BODY_BASE;
  uint32_t new_witch_resource = RESOURCE_ID_IMAGE_WITCH_WARM;
  uint32_t new_weather_resource = RESOURCE_ID_IMAGE_RAIN;
  uint32_t new_cloud_resource = RESOURCE_ID_IMAGE_CLOUDS_PARTLY;

  uint32_t *witches = needs_umbrella() ? RAINY_WITCHES : SUNNY_WITCHES;
  layer_set_hidden(bitmap_layer_get_layer(s_umbrella_layer), !needs_umbrella());

  // Conditions (body)
  if (s_body_bitmap != NULL) {
    gbitmap_destroy(s_body_bitmap);
    s_body_bitmap = NULL;
  }
  s_body_bitmap = gbitmap_create_with_resource(needs_umbrella() ? RESOURCE_ID_IMAGE_WITCH_BODY_UMBRELLA : RESOURCE_ID_IMAGE_WITCH_BODY_BASE);
  bitmap_layer_set_bitmap(s_body_layer, s_body_bitmap);
  
  // Temperature (witch)
  if (s_witch_bitmap != NULL) {
    gbitmap_destroy(s_witch_bitmap);
    s_witch_bitmap = NULL;
  }
  if (settings.TEMPERATURE >= settings.Temperature4) {
    new_witch_resource = witches[4];
  } else if (settings.TEMPERATURE >= settings.Temperature3) {
    new_witch_resource = witches[3];
  } else if (settings.TEMPERATURE >= settings.Temperature2) {
    new_witch_resource = witches[2];
  } else if (settings.TEMPERATURE >= settings.Temperature1) {
    new_witch_resource = witches[1];
  } else {
    new_witch_resource = witches[0];
  }
  s_witch_bitmap = gbitmap_create_with_resource(new_witch_resource);
  bitmap_layer_set_bitmap(s_witch_layer, s_witch_bitmap);

  // Conditions (background)
  if (s_weather_bitmap != NULL) {
    gbitmap_destroy(s_weather_bitmap);
    s_weather_bitmap = NULL;
  }
  if (needs_umbrella()) {
    if (settings.CONDITIONS == RAINY) {
      new_weather_resource = RESOURCE_ID_IMAGE_RAIN;
    } else if (settings.CONDITIONS == SNOWY) {
      new_weather_resource = RESOURCE_ID_IMAGE_SNOW;
    } else if (settings.CONDITIONS == STORMY) {
      new_weather_resource = RESOURCE_ID_IMAGE_STORM;
    }
    s_weather_bitmap = gbitmap_create_with_resource(new_weather_resource);
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
    if (settings.CONDITIONS == PARTLYCLOUDY) {
      new_cloud_resource = RESOURCE_ID_IMAGE_CLOUDS_PARTLY;
    } else {
      new_cloud_resource = RESOURCE_ID_IMAGE_CLOUDS_FULL;
    }
    s_cloud_bitmap = gbitmap_create_with_resource(new_cloud_resource);
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

static void anim_setup(Animation *animation) {

}

static void anim_update(Animation *animation, const AnimationProgress progress) {
  float_offset_y = lerp(float_start_y, float_end_y, ((float)progress) / ANIMATION_NORMALIZED_MAX);
  fly_offset_x = lerp(fly_start_x, 0, ((float)progress) / ANIMATION_NORMALIZED_MAX);
  set_witch_group_position();
}

static void anim_teardown(Animation *animation) {

}

static void fly_anim_started_handler(Animation *animation, void *context) {
  fly_offset_x = FLY_START_X;
  fly_start_x = FLY_START_X;
  float_start_y = 0;
  float_end_y = -(FLOAT_DISTANCE / 2);
  float_offset_y = float_start_y;
  set_witch_group_position();
}

static void fly_anim_stopped_handler(Animation *animation, bool finished, void *context) {
  fly_offset_x = 0;
  fly_start_x = 0;
  set_witch_group_position();
}

static Animation * create_fly_animation() {
  Animation *animation = animation_create();
  animation_set_curve(animation, AnimationCurveEaseOut);
  animation_set_duration(animation, FLY_ANIMATION_DURATION);
  animation_set_delay(animation, 500);

  const AnimationImplementation implementation = {
    .setup = anim_setup,
    .update = anim_update,
    .teardown = anim_teardown
  };
  animation_set_implementation(animation, &implementation);
  animation_set_handlers(animation, (AnimationHandlers) {
    .started = fly_anim_started_handler,
    .stopped = fly_anim_stopped_handler
  }, NULL);
  
  return animation;
}
static void float_anim_started_handler(Animation *animation, void *context) {
  int float_frame = *((int *)(context));
  if (float_frame == 0) {
    float_start_y = 0;
    float_end_y = -(FLOAT_DISTANCE / 2);
    float_offset_y = float_start_y;
  } else if (float_frame == FLOAT_COUNTS - 1) {
    float_start_y = (FLOAT_DISTANCE / 2);
    float_end_y = 0;
    float_offset_y = float_start_y;
  } else if (float_frame % 2 == 1) {
    float_start_y = -(FLOAT_DISTANCE / 2);
    float_end_y = (FLOAT_DISTANCE / 2);
    float_offset_y = float_start_y;
  } else if (float_frame % 2 == 0) {
    float_start_y = (FLOAT_DISTANCE / 2);
    float_end_y = -(FLOAT_DISTANCE / 2);
    float_offset_y = float_start_y;
  }
  set_witch_group_position();
}

static void float_anim_stopped_handler(Animation *animation, bool finished, void *context) {
  float_offset_y = float_end_y;
  set_witch_group_position();
}

static Animation * create_float_animation(int float_frame) {
  Animation *animation = animation_create();
  animation_set_curve(animation, AnimationCurveEaseInOut);
  animation_set_duration(animation, FLOAT_ANIMATION_DURATION);
  animation_set_delay(animation, 200);

  const AnimationImplementation implementation = {
    .setup = anim_setup,
    .update = anim_update,
    .teardown = anim_teardown
  };
  animation_set_implementation(animation, &implementation);
  animation_set_handlers(animation, (AnimationHandlers) {
    .started = float_anim_started_handler,
    .stopped = float_anim_stopped_handler
  }, &frames[float_frame]);

  return animation;
}

static void start_animation() {
  for (int i = 0; i < FLOAT_COUNTS; i++) {
    frames[i] = i+1;
  }

  Animation **arr = (Animation**)malloc((FLOAT_COUNTS + 1) * sizeof(Animation*));
  arr[0] = create_fly_animation();
  for (int i = 1; i < FLOAT_COUNTS; i++) {
    arr[i] = create_float_animation(i-1);
  }
  float_start_y = 0;
  float_end_y = 0;
  float_offset_y = 0;

  Animation *sequence = animation_sequence_create_from_array(arr, (FLOAT_COUNTS + 1));
  animation_set_play_count(sequence, 1);

  animation_schedule(sequence);

  free(arr);
}

static void default_settings() {  
  settings.TEMPERATURE = 80;              // placeholder temperature
  settings.CONDITIONS = RAINY;           // placeholder weather
  settings.MOON_FRACILLUM = 30;          // placeholder fracillum
  settings.MOON_WANING = true;
  settings.last_weather_received = 0;     // placeholder time

  settings.UseCurrentLocation = true;     // use GPS for weather
  settings.WeatherCheckRate = 15;         // check every 15 mins
  strcpy(settings.Latitude, "42.36");     // MIT latitude
  strcpy(settings.Longitude, "-71.1");     // MIT longitude
  settings.AmericanDate = true;           // Fri Oct 31 by default
  settings.VibrateOnDisc = true;          // vibrate by default
  
  settings.TemperatureMetric = false;        // Celsius or Fahrenheit?
  settings.Temperature0 = 30;                // Freezing temperature
  settings.Temperature1 = 50;                // Cold temperature
  settings.Temperature2 = 65;                // Chilly temperature
  settings.Temperature3 = 75;                // Warm temperature
  settings.Temperature4 = 90;                // Hot temperature
}

static void load_settings() {
  default_settings();
  // persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void request_weather() {
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

// Received data! Either for weather or settings
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Current temperature and weather conditions
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, MESSAGE_KEY_CONDITIONS);

  // if weather data is available, use it
  if (temp_tuple && conditions_tuple) {
    settings.TEMPERATURE = (int)temp_tuple->value->int32;
    settings.CONDITIONS = (Weather)conditions_tuple->value->int32;
    got_weather = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Temperature: %d - Conditions: %d", settings.TEMPERATURE, settings.CONDITIONS);
  } else { // we weren't given weather, so either settings were updated or we were poked. Request it now
    request_weather();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Requesting weather b/c of poke...");
  }

  update_time();
  update_weather();
  update_moon();
  // start_animation();
  // save_settings(); // save the new settings! Current weather included
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

  start_animation();

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
