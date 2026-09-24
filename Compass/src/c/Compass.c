#define COMPASS_WIDTH 14
#define COMPASS_HEIGHT 12

#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static Layer *s_compass_layer;
static Layer *s_needle_layer;

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

static const GColor s_colors[] = {
  GColorClear,      // = 0
  GColorWhite,      // = 1
  GColorLightGray,  // = 2
  GColorDarkGray,   // = 3
  GColorOxfordBlue, // = 4
  GColorOxfordBlue, // = 5
  GColorBlack,      // = 6
};

static const uint8_t s_pixel_thickness = 10;

static int32_t s_angle = 90;

static void compass_update_proc(Layer *layer, GContext *ctx) { 
  //draw pixels
  for (size_t h = 0; h < COMPASS_HEIGHT; h++)
  {
    for (size_t w = 0; w < COMPASS_WIDTH; w++)
    {
      uint8_t color_nr = s_compass[(h * COMPASS_WIDTH) + w];
      GColor color = s_colors[color_nr];
      if (color_nr != 0)
      {
        graphics_context_set_fill_color(ctx, color);
        graphics_fill_rect(ctx, GRect(w * s_pixel_thickness, h * s_pixel_thickness, s_pixel_thickness, s_pixel_thickness), 0, GCornerNone);
      }
    }
  }
}

static void needle_update_proc(Layer *layer, GContext *ctx) { 
  // based on: https://minecraft.wiki/w/Procedural_animated_texture_generation/Compasses
  int32_t angle = DEG_TO_TRIGANGLE(s_angle);

  const float NX = 7.5f; //8.5
  const float NY = 5.5f; //7.5
  const float SCALE_X = 0.3f;
  const float SCALE_Y = SCALE_X * 0.5f;

  float rx = (float)sin_lookup(angle) / 65536.0f;
  float ry = (float)cos_lookup(angle) / 65536.0f;

  // draw the smaller horizontal spurs of the needle
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  for (int8_t i = -4; i <= 4; i++)
  {
    int8_t x = NX + ry * i * SCALE_X;
    int8_t y = NY - rx * i * SCALE_Y;
    graphics_fill_rect(ctx, GRect(x * s_pixel_thickness, y * s_pixel_thickness, s_pixel_thickness, s_pixel_thickness), 0, GCornerNone);
  }

  // draw the main part needle
  for (int8_t i = -8; i <= 16; i++)
  {
    int8_t x = NX + rx * i * SCALE_X;
    int8_t y = NY + ry * i * SCALE_Y;
    if (i >= 0) {
      // main red pointer
      graphics_context_set_fill_color(ctx, /*GColorRed*/GColorWhite);
    } else {
      // grey back half
      graphics_context_set_fill_color(ctx, GColorDarkGray);
    }
    graphics_fill_rect(ctx, GRect(x * s_pixel_thickness, y * s_pixel_thickness, s_pixel_thickness, s_pixel_thickness), 0, GCornerNone);
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
  uint8_t width_compass = (COMPASS_WIDTH * s_pixel_thickness);
  uint8_t height_compass = (COMPASS_HEIGHT * s_pixel_thickness);
  uint8_t x0_compass = (PBL_DISPLAY_WIDTH - width_compass) / 2;
  uint8_t y0_compass = (PBL_DISPLAY_HEIGHT - height_compass) / 2;

  // text layer
  s_text_layer = text_layer_create(GRect(0, y0_compass + height_compass + 5, bounds.size.w, 20));
  text_layer_set_text(s_text_layer, "Press a button");
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
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_compass_layer);
  layer_destroy(s_needle_layer);
  text_layer_destroy(s_text_layer);
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
