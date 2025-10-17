#include <pebble.h>
#include "structs.h"
#include "defines.h"

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

static float squared(float x) {
  return x*x;
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
  // Fracillum %s:
  // < 5% -> new
  // > 95% -> full
  // waning: left side is white (-180 -> 0)
  // waxing: right side is white (0 -> 180)
  // waning: report negative %
  // waxing: report positive %
  // if % < 50: color = black else white
  // frac width = % * bounds.width/2?
  // if % > 50: width = (%-50) * bounds.width / 2
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
  layer_mark_dirty(s_moon_layer);
}

static void default_settings() {  
  settings.TEMPERATURE = 80;              // placeholder temperature
  settings.CONDITIONS = PARTLYCLOUDY;           // placeholder weather
  settings.MOON_FRACILLUM = 30;          // placeholder fracillum
  settings.MOON_WANING = true;
  settings.last_weather_received = 0;     // placeholder time

  settings.UseCurrentLocation = true;     // use GPS for weather
  settings.WeatherCheckRate = 15;         // check every 15 mins
  strcpy(settings.Latitude, "42.36");     // MIT latitude
  strcpy(settings.Latitude, "-71.1");     // MIT longitude
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

// setup the display
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // stars: 0, 0
  s_stars_layer = bitmap_layer_create(GRect(0 + X_OFFSET, 0 + Y_OFFSET, 180, 57));
  s_stars_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STARS);
  bitmap_layer_set_bitmap(s_stars_layer, s_stars_bitmap);
  
  // weather: 0, 48
  s_weather_layer = bitmap_layer_create(GRect(0 + X_OFFSET, 48 + Y_OFFSET, 180, 132));
  s_weather_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_RAIN);
  bitmap_layer_set_bitmap(s_weather_layer, s_weather_bitmap);
  
  // moon: 73, 19
  s_moon_bm_layer = bitmap_layer_create(GRect(73 + X_OFFSET, 19 + Y_OFFSET, 34, 33));
  s_moon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOON);
  bitmap_layer_set_bitmap(s_moon_bm_layer, s_moon_bitmap);
  bitmap_layer_set_compositing_mode(s_moon_bm_layer, GCompOpSet);
  // Real moon: TODO
  s_moon_layer = layer_create(GRect(73 + X_OFFSET, 19 + Y_OFFSET, MOON_SIZE, MOON_SIZE));
  layer_set_update_proc(s_moon_layer, moon_update_proc);
  // s_battery_layer = layer_create(GRect(121 + X_OFFSET, 52 + Y_OFFSET, 22, 22));

  // clouds: 0, 0
  s_cloud_layer = bitmap_layer_create(GRect(0 + X_OFFSET, 0 + Y_OFFSET, 180, 57));
  s_cloud_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CLOUDS_FULL);
  bitmap_layer_set_bitmap(s_cloud_layer, s_cloud_bitmap);
  bitmap_layer_set_compositing_mode(s_cloud_layer, GCompOpSet);

  // time: x, 108 ish
  s_time_layer = text_layer_create(GRect(0, 108 + Y_OFFSET, bounds.size.w, 38));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_TIME_34));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  // date: x, 144 ish
  s_date_layer = text_layer_create(GRect(0, 144 + Y_OFFSET, bounds.size.w, 24));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_DATE_20));
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorWhite);
  text_layer_set_font(s_date_layer, s_date_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  // broom: 50, 66
  s_broom_layer = bitmap_layer_create(GRect(50 + X_OFFSET, 66 + Y_OFFSET, 81, 52));
  s_broom_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BROOM);
  bitmap_layer_set_bitmap(s_broom_layer, s_broom_bitmap);
  bitmap_layer_set_compositing_mode(s_broom_layer, GCompOpSet);

  // Body: 67, 70
  s_body_layer = bitmap_layer_create(GRect(67 + X_OFFSET, 70 + Y_OFFSET, 37, 47));
  s_body_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WITCH_BODY_UMBRELLA);
  bitmap_layer_set_bitmap(s_body_layer, s_body_bitmap);
  bitmap_layer_set_compositing_mode(s_body_layer, GCompOpSet);

  // Witch: 67, 70
  s_witch_layer = bitmap_layer_create(GRect(67 + X_OFFSET, 70 + Y_OFFSET, 37, 47));
  s_witch_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_WITCH_WARM_UMBRELLA);
  bitmap_layer_set_bitmap(s_witch_layer, s_witch_bitmap);
  bitmap_layer_set_compositing_mode(s_witch_layer, GCompOpSet);
  
  // Cat: 102, 89
  s_cat_layer = bitmap_layer_create(GRect(102 + X_OFFSET, 89 + Y_OFFSET, 22, 14));
  s_cat_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CAT_LOAFING);
  bitmap_layer_set_bitmap(s_cat_layer, s_cat_bitmap);
  bitmap_layer_set_compositing_mode(s_cat_layer, GCompOpSet);

  // umbrella: 74, 63
  s_umbrella_layer = bitmap_layer_create(GRect(74 + X_OFFSET, 63 + Y_OFFSET, 52, 35));
  s_umbrella_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UMBRELLA);
  bitmap_layer_set_bitmap(s_umbrella_layer, s_umbrella_bitmap);
  bitmap_layer_set_compositing_mode(s_umbrella_layer, GCompOpSet);
  

  layer_add_child(window_layer, bitmap_layer_get_layer(s_stars_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_weather_layer));
  // layer_add_child(window_layer, bitmap_layer_get_layer(s_moon_bm_layer));
  layer_add_child(window_layer, s_moon_layer);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_cloud_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_broom_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_body_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_witch_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_cat_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_umbrella_layer));
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

// // update the time display
// static void update_time() {
// #ifdef DEMO_MODE
//   text_layer_set_text(s_hour_layer, DEMO_CYCLE ? s_demo_hours[DEMO_CYCLE_POS] : DEMO_HOUR);
//   text_layer_set_text(s_colon_layer, ":");
//   text_layer_set_text(s_minute_layer, DEMO_CYCLE ? s_demo_minutes[DEMO_CYCLE_POS] : DEMO_MINUTE);
// #else
//   time_t temp = time(NULL);
//   struct tm *tick_time = localtime(&temp);

//   // put hours and minutes into buffer
//   static char s_hour_buffer[8];
//   strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ?
//                                         "%H" : "%I", tick_time);
//   text_layer_set_text(s_hour_layer, s_hour_buffer); 
  
//   text_layer_set_text(s_colon_layer, ":");

//   static char s_minute_buffer[8];
//   strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);
//   text_layer_set_text(s_minute_layer, s_minute_buffer);  
// #endif
// }

// static void update_date(struct tm *tick_time){
// #ifdef DEMO_MODE
//   if (DEMO_CYCLE) {
//     text_layer_set_text(s_date_layer, s_demo_dates[DEMO_CYCLE_POS]);
//     text_layer_set_text(s_day_layer, s_demo_days[DEMO_CYCLE_POS]);
    
//     gbitmap_destroy(s_day_icon_bitmap);
//     s_day_icon_bitmap = gbitmap_create_with_resource(DAY_ICONS[s_demo_day_icons[DEMO_CYCLE_POS]]);
//     bitmap_layer_set_bitmap(s_day_icon_layer, s_day_icon_bitmap);
    
//   } else {
//     text_layer_set_text(s_date_layer, DEMO_DATE);
//     text_layer_set_text(s_day_layer, DEMO_DAY);
    
//     gbitmap_destroy(s_day_icon_bitmap);
//     s_day_icon_bitmap = gbitmap_create_with_resource(DAY_ICONS[DEMO_DAY_ICON]);
//     bitmap_layer_set_bitmap(s_day_icon_layer, s_day_icon_bitmap);
//   }
// #else
//   static char s_date_buffer[8];
//   if (settings.AmericanDate) {
//     strftime(s_date_buffer, sizeof(s_date_buffer), "%b %d", tick_time); // displayed as "Jan 01"
//   } else {
//     strftime(s_date_buffer, sizeof(s_date_buffer), "%d %b", tick_time); // displayed as "01 Jan"
//   }

//   text_layer_set_text(s_date_layer, s_date_buffer);

//   static char s_day_buffer[8];
//   strftime(s_day_buffer, sizeof(s_day_buffer), "%a", tick_time);
//   text_layer_set_text(s_day_layer, s_day_buffer);

//   gbitmap_destroy(s_day_icon_bitmap);
//   s_day_icon_bitmap = gbitmap_create_with_resource(DAY_ICONS[tick_time->tm_wday]);
//   bitmap_layer_set_bitmap(s_day_icon_layer, s_day_icon_bitmap);
// #endif

//   refresh_colors();
// }

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

  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());

  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });

  // // callback for bluetooth connection updates
  // connection_service_subscribe((ConnectionHandlers) {
  //   .pebble_app_connection_handler = bluetooth_callback
  // });

  // // Register callbacks for settings/weather updates
  // app_message_register_inbox_received(inbox_received_callback);
  // app_message_register_inbox_dropped(inbox_dropped_callback);
  // app_message_register_outbox_failed(outbox_failed_callback);
  // app_message_register_outbox_sent(outbox_sent_callback);

  // // Open AppMessage
  // const int inbox_size = 1024; // maaaaybe overkill, but 128 isn't enough
  // const int outbox_size = 1024;
  // app_message_open(inbox_size, outbox_size);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
