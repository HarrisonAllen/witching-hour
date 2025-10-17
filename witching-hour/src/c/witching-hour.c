#include <pebble.h>
#include "structs.h"

#define Y_OFFSET (PBL_DISPLAY_HEIGHT - 180) / 2
#define X_OFFSET (PBL_DISPLAY_WIDTH - 180) / 2

// Window
static Window *s_main_window;
// Layers
static TextLayer *s_time_layer, *s_date_layer;
static BitmapLayer *s_stars_layer, *s_moon_layer, *s_cloud_layer, *s_weather_layer, *s_broom_layer, *s_witch_layer, *s_cat_layer, *s_umbrella_layer;
// static Layer *s_moon_layer;
// Resources
static GFont s_time_font, s_date_font;
static GBitmap *s_stars_bitmap, *s_moon_bitmap, *s_cloud_bitmap, *s_weather_bitmap, *s_broom_bitmap, *s_witch_bitmap, *s_cat_bitmap, *s_umbrella_bitmap;
// Globals

// update the batter display layer
static void battery_callback(BatteryChargeState state) {
  uint32_t new_cat_resource = RESOURCE_ID_IMAGE_CAT_STANDING;

  if (s_cat_bitmap != NULL) {
    gbitmap_destroy(s_cat_bitmap);
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
  s_moon_layer = bitmap_layer_create(GRect(73 + X_OFFSET, 19 + Y_OFFSET, 34, 33));
  s_moon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MOON);
  bitmap_layer_set_bitmap(s_moon_layer, s_moon_bitmap);
  bitmap_layer_set_compositing_mode(s_moon_layer, GCompOpSet);
  // Real moon: TODO
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
  layer_add_child(window_layer, bitmap_layer_get_layer(s_moon_layer));
  // layer_add_child(window_layer, s_moon_layer);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_cloud_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_broom_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_witch_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_cat_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_umbrella_layer));

  // TODO: remove
  text_layer_set_text(s_time_layer, "22:45");
  text_layer_set_text(s_date_layer, "Mon Oct 13");
}

// unload everything!
static void main_window_unload(Window *window) {
  // unload regular layers
  // if (s_moon_layer != NULL)
  //   layer_destroy(s_moon_layer);

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
  if (s_moon_layer != NULL)
    bitmap_layer_destroy(s_moon_layer);
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
  // load_settings();

  // setup window
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_set_background_color(s_main_window, GColorBlack);

  window_stack_push(s_main_window, true);

  // // set up tick_handler to run every minute
  // tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  // // want to display time and date at the start
  // update_time();
  // time_t temp = time(NULL);
  // struct tm *tick_time = localtime(&temp);
  // update_date(tick_time);

  battery_state_service_subscribe(battery_callback);
  battery_callback(battery_state_service_peek());

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
