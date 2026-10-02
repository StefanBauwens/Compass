#define COMPASS_WIDTH 14
#define COMPASS_HEIGHT 12
#define STEP_DELAY 150 //ms
#define PIXEL_THICKNESS 10

#include <pebble.h>
#include "libraries/sdith.h"
#include "libraries/miniwindow.h"

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
        sdith_fill_rect(ctx, GRect(w * PIXEL_THICKNESS, h * PIXEL_THICKNESS, PIXEL_THICKNESS, PIXEL_THICKNESS), 0, GCornerNone);
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
    sdith_fill_rect(ctx, s_recorded_draw_commands[i].rect, 0, GCornerNone);
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
      text_layer_set_text(s_text_layer, "Not yet calibrated...");
      break;
    case CompassStatusUnavailable:
      text_layer_set_text(s_text_layer, "No compass avaialble!");
      break;
    case CompassStatusCalibrating:
      text_layer_set_text(s_text_layer, "Calibrating...");
      s_angle = TRIGANGLE_TO_DEG(TRIG_MAX_ANGLE - heading_data.magnetic_heading) + 180;
      record_needle_draw_commands(); // do heavy lifting here so it doesn't happen in draw command
      layer_mark_dirty(s_needle_layer);
      break;
    case CompassStatusCalibrated:
      text_layer_set_text(s_text_layer, "Callibrated!");
      s_angle = TRIGANGLE_TO_DEG(TRIG_MAX_ANGLE - heading_data.magnetic_heading) + 180;
      record_needle_draw_commands(); // do heavy lifting here so it doesn't happen in draw command
      layer_mark_dirty(s_needle_layer);
      break;
  }
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Select");
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_angle+=1;
  layer_mark_dirty(s_needle_layer);
  static char progress_text[25];
  snprintf(progress_text, sizeof(progress_text), "angle %ld", s_angle);
  text_layer_set_text(s_text_layer, progress_text);
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  s_angle-=1;
  layer_mark_dirty(s_needle_layer);
  static char progress_text[25];
  snprintf(progress_text, sizeof(progress_text), "Angle %ld", s_angle);
  text_layer_set_text(s_text_layer, progress_text);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // calculate values for compass
  uint8_t width_compass = (COMPASS_WIDTH * PIXEL_THICKNESS);
  uint8_t height_compass = (COMPASS_HEIGHT * PIXEL_THICKNESS);
  uint8_t x0_compass = (PBL_DISPLAY_WIDTH - width_compass) / 2;
  uint8_t y0_compass = (PBL_DISPLAY_HEIGHT - height_compass) / 2;

  // text layer
  s_text_layer = text_layer_create(GRect(0, y0_compass + height_compass + 5, bounds.size.w, 20));
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
  create_mini_window(window_layer, "title", "this is a body value");

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

  const bool animated = true;
  window_stack_push(s_window, animated);
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
