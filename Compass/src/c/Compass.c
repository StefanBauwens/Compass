#define COMPASS_WIDTH 14
#define COMPASS_HEIGHT 12
#define STEP_DELAY 150 //ms
#define PIXEL_THICKNESS 10
#define SETTINGS_KEY 1
#define CUSTOM_LOCATIONS_COUNT 5

#include <pebble.h>
#include "libraries/sdith.h"
#include "libraries/miniwindow.h"

static void s_set_heading_items();
static void s_select_heading(int index);

typedef struct {
    DitherTemplate dither_template;
    GRect rect;
} DitherRect; // used for recording draw commands

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_compass_layer;
static Layer *s_needle_layer;

//ticks
static AppTimer *milli_tick_handler;

static const uint8_t s_compass[] = {
  0,0,0,0,4,4,4,4,4,4,0,0,0,0,
  0,0,4,4,3,2,2,2,2,2,4,4,0,0,
  0,4,3,3,5,5,5,5,5,5,2,1,4,0,
  4,3,5,5,5,5,5,5,5,5,5,5,2,6,
  4,5,5,5,5,5,5,5,5,5,5,5,5,6,
  4,5,5,5,5,5,5,5,5,5,5,5,5,6,
  4,6,5,5,5,5,5,5,5,5,5,5,6,6,
  4,2,6,6,5,5,5,5,5,5,6,6,3,6,
  4,2,1,2,6,6,6,6,6,6,3,3,3,6,
  0,6,1,2,2,2,2,2,2,3,3,3,6,0,
  0,0,6,6,2,2,2,2,2,3,6,6,0,0,
  0,0,0,0,6,6,6,6,6,6,0,0,0,0
};

/* CONFIG */
typedef struct ClaySettings {
  bool UseMetricSystem;
  char Location1Name[20];
  char Location2Name[20];
  char Location3Name[20];
  char Location4Name[20];
  char Location5Name[20];
} ClaySettings;

static ClaySettings settings = {0};

static void save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  s_set_heading_items();
}

static void load_settings() {
  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
    s_set_heading_items();
  }
}
/* END CONFIG */

/* HEADING MENU */
static Window *s_heading_menu_window;
static SimpleMenuLayer *s_heading_menu_layer;

static void heading_menu_select_callback(int index, void *context) {
  switch (index) {
    case 0:
      //TODO make all headings actually work
      s_select_heading(0);
      break;
    case 1:
      if (settings.Location1Name[0] != '\0')
      {
        s_select_heading(1);
      }
      break;
    case 2:
      if (settings.Location2Name[0] != '\0')
      {
        s_select_heading(2);
      }
      break;
    case 3:
      if (settings.Location3Name[0] != '\0')
      {
        s_select_heading(3);
      }
      break;
    case 4:
      if (settings.Location4Name[0] != '\0')
      {
        s_select_heading(4);
      }
      break;
    case 5:
      if (settings.Location5Name[0] != '\0')
      {
        s_select_heading(5);
      }
      break;
  }
}

static SimpleMenuItem s_heading_menu_items[6] = {
  { .title = "North", .subtitle = "Selected", .callback = heading_menu_select_callback },
  { .title = "Empty", .callback = heading_menu_select_callback }, // filled in later
  { .title = "Empty", .callback = heading_menu_select_callback },
  { .title = "Empty", .callback = heading_menu_select_callback },
  { .title = "Empty", .callback = heading_menu_select_callback },
  { .title = "Empty", .callback = heading_menu_select_callback }
};

static void s_select_heading(int index) {
  for (int i = 0; i < CUSTOM_LOCATIONS_COUNT + 1; i++) // + 1 for north
  {
    if (i == index) {
      s_heading_menu_items[i].subtitle = "Selected";
    } else {
      s_heading_menu_items[i].subtitle = "";
    }
  }
  layer_mark_dirty(simple_menu_layer_get_layer(s_heading_menu_layer));
}

static SimpleMenuSection s_heading_menu_sections[] = {
  {
    .title = NULL,
    .items = s_heading_menu_items,
    .num_items = 6 
  }
};

static void s_set_heading_items() {
    if (settings.Location1Name[0] != '\0') {
      s_heading_menu_items[1].title = settings.Location1Name;
    } else {
      s_heading_menu_items[1].title = "Empty";
    }

    if (settings.Location2Name[0] != '\0') {
      s_heading_menu_items[2].title = settings.Location2Name;
    } else {
      s_heading_menu_items[2].title = "Empty";
    }

    if (settings.Location3Name[0] != '\0') {
      s_heading_menu_items[3].title = settings.Location3Name;
    } else {
      s_heading_menu_items[3].title = "Empty";
    }

    if (settings.Location4Name[0] != '\0') {
      s_heading_menu_items[4].title = settings.Location4Name;
    } else {
      s_heading_menu_items[4].title = "Empty";
    }

    if (settings.Location5Name[0] != '\0') {
      s_heading_menu_items[5].title = settings.Location5Name;
    } else {
      s_heading_menu_items[5].title = "Empty";
    }
}

static void heading_menu_window_load(Window *window) {
  s_heading_menu_layer = simple_menu_layer_create(
    layer_get_bounds(window_get_root_layer(window)),
    window,
    s_heading_menu_sections,
    1,
    NULL
  );

  layer_add_child(
    window_get_root_layer(window),
    simple_menu_layer_get_layer(s_heading_menu_layer)
  );
}

static void heading_menu_window_unload(Window *window) {
  simple_menu_layer_destroy(s_heading_menu_layer);
  s_heading_menu_layer = NULL;
}

/* END HEADING MENU */

/* MAIN MENU */
static Window *s_main_menu_window;
static SimpleMenuLayer *s_main_menu_layer;

static void main_menu_select_callback(int index, void *context) {
  switch (index) {
    case 0:
      // open submenu
      window_stack_push(s_heading_menu_window, true);
      break;

    case 1:
      show_mini_window();
      window_stack_pop(false);
      break;
  }
}

static SimpleMenuItem s_main_menu_items[3] = {
  { .title = "Set Heading", .callback = main_menu_select_callback },
  { .title = "Calibration Info", .callback = main_menu_select_callback },
};

static SimpleMenuSection s_main_menu_sections[] = {
  {
    .title = NULL,
    .items = s_main_menu_items,
    .num_items = 2
  }
};

static void main_menu_window_load(Window *window) {
  s_main_menu_layer = simple_menu_layer_create(
    layer_get_bounds(window_get_root_layer(window)),
    window,
    s_main_menu_sections,
    1,
    NULL
  );

  layer_add_child(
    window_get_root_layer(window),
    simple_menu_layer_get_layer(s_main_menu_layer)
  );
}

static void main_menu_window_unload(Window *window) {
  simple_menu_layer_destroy(s_main_menu_layer);
  s_main_menu_layer = NULL;
}

/* END MAIN MENU */

#if defined(PBL_COLOR)
static const DitherTemplate s_color_palette[] = {
  COLOR_TEMPLATE(GColorClear),                              // = 0
  DITHER_TEMPLATE(DITHER_8, GColorWhite, GColorLightGray),  // = 1
  COLOR_TEMPLATE(GColorLightGray),                          // = 2
  COLOR_TEMPLATE(GColorDarkGray),                           // = 3
  DITHER_TEMPLATE(DITHER_13, GColorDarkGray, GColorBlack),  // = 4
  DITHER_TEMPLATE(DITHER_14, GColorDarkGray, GColorBlack),  // = 5
  COLOR_TEMPLATE(GColorBlack),                              // = 6
  COLOR_TEMPLATE(GColorRed)                                 // = 7
};
#else // BLACK AND WHITE MODELS
static const DitherTemplate s_color_palette[] = {
  COLOR_TEMPLATE(GColorClear),                              // = 0
  DITHER_TEMPLATE(DITHER_1, GColorWhite, GColorBlack),      // = 1
  DITHER_TEMPLATE(DITHER_6, GColorWhite, GColorBlack),      // = 2
  DITHER_TEMPLATE(DITHER_8, GColorWhite, GColorBlack),      // = 3
  DITHER_TEMPLATE(DITHER_14, GColorWhite, GColorBlack),     // = 4
  DITHER_TEMPLATE(DITHER_15, GColorWhite, GColorBlack),     // = 5
  COLOR_TEMPLATE(GColorBlack),                              // = 6
  DITHER_TEMPLATE(DITHER_4, GColorWhite, GColorBlack),      // = 7
};
#endif

static const int16_t s_clamp_angles[] = { 0, 11, 21, 36, 43, 54, 66, 90, 113, 126, 137, 143, 158, 169, 180, 191, 201, 216, 223, 234, 246, 270, 293, 306, 317, 324, 338, 349, 360 };  // angles for the compass to clamp to that corresponds to the 28 pre-rendered sprite of Minecraft Compass.

static int32_t s_angle = 90;

static DitherRect s_recorded_draw_commands[34];

static void compass_update_proc(Layer *layer, GContext *ctx) { 
  //draw pixels
  for (size_t h = 0; h < COMPASS_HEIGHT; h++)
  {
    for (size_t w = 0; w < COMPASS_WIDTH; w++)
    {
      uint8_t color_nr = s_compass[(h * COMPASS_WIDTH) + w];
      DitherTemplate color = s_color_palette[color_nr];
      if (color_nr != 0)
      {
        sdith_set_dither_template(color);
        sdith_fill_rect(layer, ctx, GRect(w * PIXEL_THICKNESS, h * PIXEL_THICKNESS, PIXEL_THICKNESS, PIXEL_THICKNESS), 0, GCornerNone);
      }
    }
  }
}

static int positive_modulo(int value, int mod)
{
  return ((value % mod) + mod) % mod;
}

static size_t get_clamped_angle_index(int32_t angle)
{
  // first map the angle to 0-360 correctly
  int32_t clean_angle = positive_modulo(s_angle, 360);//((s_angle % 360) + 360) % 360;

  // clamp angle using the 28 values.
  size_t index = 0;
  size_t array_length = ARRAY_LENGTH(s_clamp_angles);
  for (size_t i = 0; i < array_length; i++)
  {
    if (clean_angle <= s_clamp_angles[i])
    {
      if (i == 0) { // if first item, clamp to it (== 0/360)
        index = i;
        //clamped_angle = s_clamp_angles[i];
      } else { // i = 1 or more
        // check if closer to this or previous value
        if (s_clamp_angles[i] - clean_angle < clean_angle - s_clamp_angles[i-1]) {
          index = i;
          //clamped_angle = s_clamp_angles[i];
        } else{
          index = i - 1;
          //clamped_angle = s_clamp_angles[i-1];
        }
      }
      break; // exit loop
    }
  }

  return index;
}

static void record_needle_draw_commands()
{
  // based on: https://minecraft.wiki/w/Procedural_animated_texture_generation/Compasses
  const float NX = 7.5f; //8.5
  const float NY = 5.5f; //7.5
  const float SCALE_X = 0.3f;
  const float SCALE_Y = SCALE_X * 0.5f;

  int32_t angle = DEG_TO_TRIGANGLE(s_clamp_angles[get_clamped_angle_index(s_angle)]);
  
  float rx = (float)sin_lookup(angle) / 65536.0f;
  float ry = (float)cos_lookup(angle) / 65536.0f;

  size_t recording_index = 0;

  // draw the smaller horizontal spurs of the needle
  for (int8_t i = -4; i <= 4; i++)
  {
    int8_t x = NX + ry * i * SCALE_X;
    int8_t y = NY - rx * i * SCALE_Y;
    s_recorded_draw_commands[recording_index] = (DitherRect){
      .dither_template = s_color_palette[3], 
      .rect = GRect(x * PIXEL_THICKNESS, y * PIXEL_THICKNESS, PIXEL_THICKNESS, PIXEL_THICKNESS) 
    };
    recording_index++;
  }

  // draw the main part needle
  for (int8_t i = -8; i <= 16; i++)
  {
    int8_t x = NX + rx * i * SCALE_X;
    int8_t y = NY + ry * i * SCALE_Y;
    s_recorded_draw_commands[recording_index] = (DitherRect){ 
      .dither_template = (i >= 0 ? s_color_palette[7] : s_color_palette[3]), 
      .rect = GRect(x * PIXEL_THICKNESS, y * PIXEL_THICKNESS, PIXEL_THICKNESS, PIXEL_THICKNESS) 
    };
    recording_index++;
  }
}

static void needle_update_proc(Layer *layer, GContext *ctx) 
{ 
  // run draw commands
  for (size_t i = 0; i < ARRAY_LENGTH(s_recorded_draw_commands); i++)
  {
    sdith_set_dither_template(s_recorded_draw_commands[i].dither_template);
    sdith_fill_rect(layer, ctx, s_recorded_draw_commands[i].rect, 0, GCornerNone);
  }
}

static void milli_tick() //runs once every ms.
{
  s_angle = positive_modulo(s_angle - 5, 360);
  record_needle_draw_commands(); // do heavy lifting here so it doesn't happen in draw command
  layer_mark_dirty(s_needle_layer);
  milli_tick_handler = app_timer_register(STEP_DELAY, milli_tick, NULL); // calls itself in STEP_DELAY ms
}

static void compass_heading_handler(CompassHeadingData heading_data) {
  // Is the compass calibrated?
  switch(heading_data.compass_status) {
    case CompassStatusDataInvalid:
      text_layer_set_text(s_text_layer, "❎");
      break;
    case CompassStatusUnavailable:
      text_layer_set_text(s_text_layer, "❗");
      break;
    case CompassStatusCalibrating:
      text_layer_set_text(s_text_layer, "C");
      s_angle = TRIGANGLE_TO_DEG(TRIG_MAX_ANGLE - heading_data.magnetic_heading) + 180;
      record_needle_draw_commands(); // do heavy lifting here so it doesn't happen in draw command
      layer_mark_dirty(s_needle_layer);
      break;
    case CompassStatusCalibrated:
      //text_layer_set_text(s_text_layer, "✅");
      s_angle = TRIGANGLE_TO_DEG(TRIG_MAX_ANGLE - heading_data.magnetic_heading) + 180;
      record_needle_draw_commands(); // do heavy lifting here so it doesn't happen in draw command
      layer_mark_dirty(s_needle_layer);
      break;
  }
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {

  bool saveSettings = false;
  Tuple *use_metric_t = dict_find(iter, MESSAGE_KEY_UseMetricSystem);
  if(use_metric_t) {
    settings.UseMetricSystem = use_metric_t->value->int32 == 1;
    saveSettings = true;
  }
  
  Tuple *location_1_name_t = dict_find(iter, MESSAGE_KEY_Location1Name);
  if (location_1_name_t) {
    snprintf(settings.Location1Name, sizeof(settings.Location1Name), "%s", location_1_name_t->value->cstring);
    saveSettings = true;
  }

  Tuple *location_2_name_t = dict_find(iter, MESSAGE_KEY_Location2Name);
  if (location_2_name_t) {
    snprintf(settings.Location2Name, sizeof(settings.Location2Name), "%s", location_2_name_t->value->cstring);
    saveSettings = true;
  }

  Tuple *location_3_name_t = dict_find(iter, MESSAGE_KEY_Location3Name);
  if (location_3_name_t) {
    snprintf(settings.Location3Name, sizeof(settings.Location3Name), "%s", location_3_name_t->value->cstring);
    saveSettings = true;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Character: '%c'", settings.Location3Name[0]);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "ASCII value: %d", (unsigned char)settings.Location3Name[0]);
  }

  Tuple *location_4_name_t = dict_find(iter, MESSAGE_KEY_Location4Name);
  if (location_4_name_t) {
    snprintf(settings.Location4Name, sizeof(settings.Location4Name), "%s", location_4_name_t->value->cstring);
    saveSettings = true;
  }

  Tuple *location_5_name_t = dict_find(iter, MESSAGE_KEY_Location5Name);
  if (location_5_name_t) {
    snprintf(settings.Location5Name, sizeof(settings.Location5Name), "%s", location_5_name_t->value->cstring);
    saveSettings = true;
  }

  if (saveSettings)
  {
    save_settings();
  }

  //TODO save the long and lat? OR consider doing some of the calculations on the js end? just pass the relative angle to north? (need to be polled of course if we're closer). Either way we need to "pol" because we need to update our location so we might as well do the heavy math on js end
 
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  hide_mini_window();
  window_stack_push(s_main_menu_window, true); 
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  //hide_mini_window();
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  //hide_mini_window();
}

static void prv_back_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (is_showing_mini_window())
  {
    hide_mini_window();
  }
  else
  {
    window_stack_pop(true);
  }
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
  window_single_click_subscribe(BUTTON_ID_BACK, prv_back_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Load persistant settings
  load_settings();

  // calculate values for compass
  uint8_t width_compass = (COMPASS_WIDTH * PIXEL_THICKNESS);
  uint8_t height_compass = (COMPASS_HEIGHT * PIXEL_THICKNESS);
  uint8_t x0_compass = (PBL_DISPLAY_WIDTH - width_compass) / 2;
  uint8_t y0_compass = (PBL_DISPLAY_HEIGHT - height_compass) / 2;

  // text layer
  s_text_layer = text_layer_create(GRect(0, 0, 15, 15));
  //text_layer_set_text(s_text_layer, "Press a button");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  // create compass background layer
  s_compass_layer = layer_create(GRect(x0_compass, y0_compass, width_compass, height_compass));
  layer_set_update_proc(s_compass_layer, compass_update_proc);
  layer_add_child(window_layer, s_compass_layer);

  // create needle layer
  s_needle_layer = layer_create(GRect(x0_compass, y0_compass, width_compass, height_compass));
  layer_set_update_proc(s_needle_layer, needle_update_proc);
  layer_add_child(window_layer, s_needle_layer);

  // Sub to ticks
  //milli_tick_handler = app_timer_register(STEP_DELAY, milli_tick, NULL);

  // mini window
  DitherTemplate background = DITHER_TEMPLATE(DITHER_BLANK, GColorWhite, GColorWhite);
  create_mini_window(window_layer, GRect(10, 10, 124, 134), "Calibration Info", "❎ Not calibrated\n C Busy Calibrating\n\nCalibrate watch by moving in an 8 figure till no symbol is visible.", background);

  // Subscribe to compass heading updates
  compass_service_subscribe(compass_heading_handler); 
  // Only notify when the heading has changed by more than 5 degrees.
  compass_service_set_heading_filter(DEG_TO_TRIGANGLE(5));
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_compass_layer);
  layer_destroy(s_needle_layer);
  text_layer_destroy(s_text_layer);
  
  destroy_mini_window();

  // Unsubscribe to ticks
  app_timer_cancel(milli_tick_handler);

  // unsub from compass
  compass_service_unsubscribe();
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });

  s_main_menu_window = window_create();  
  window_set_window_handlers(s_main_menu_window, (WindowHandlers) {
    .load = main_menu_window_load,
    .unload = main_menu_window_unload
  });

  s_heading_menu_window = window_create();  
  window_set_window_handlers(s_heading_menu_window, (WindowHandlers) {
    .load = heading_menu_window_load,
    .unload = heading_menu_window_unload
  });

  load_settings();

  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);

  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
  window_destroy(s_main_menu_window);
  window_destroy(s_heading_menu_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
