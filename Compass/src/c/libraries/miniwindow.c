#define MINI_WINDOW_TITLE_HEIGHT 20
#define MINI_WINDOW_TITLE_PADDING_HORIZONTAL 6
#define MINI_WINDOW_PADDING_TOP 1
#define MINI_WINDOW_BODY_MARGIN_HORIZONTAL 5
#define MINI_WINDOW_BODY_MARGIN_VERTICAL 2

#include "miniwindow.h"

static TextLayer *s_title_layer;
static TextLayer *s_text_body_layer;
static Layer *s_main_layer;
static Layer *s_window_layer;
static bool s_is_created = false;
static bool s_is_showing = false;
static GRect s_window_frame;
static GRect s_title_frame;
static GRect s_body_frame;

static DitherTemplate bg_dith;

void main_layer_update_proc(Layer *layer, GContext *ctx) { 

    // draw window background
    sdith_set_dither_template(bg_dith);
    sdith_fill_rect(layer, ctx, GRect(2, MINI_WINDOW_TITLE_HEIGHT, s_window_frame.size.w-4, s_window_frame.size.h - MINI_WINDOW_TITLE_HEIGHT- 1), 7, GCornersBottom);

    // draw white outline of window
    
    graphics_context_set_stroke_width(ctx, 1); 
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_round_rect(ctx, GRect(0, 0, s_window_frame.size.w, s_window_frame.size.h), 8);

    // draw black (inner)window of window
    graphics_context_set_stroke_width(ctx, 2);
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_round_rect(ctx, GRect(2, 2, s_window_frame.size.w-4, s_window_frame.size.h-4), 7);

    // draw top part of window
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(2, 2, s_window_frame.size.w-4, MINI_WINDOW_TITLE_HEIGHT - 2), 7, GCornersTop);
}

void create_mini_window(Layer * window_layer, GRect rect, const char * title, const char * body, DitherTemplate background)
{
    if (s_is_created)
    {
        return;
    }

    bg_dith = background;
    s_window_layer = window_layer;
    
    s_window_frame = rect;
    s_title_frame = GRect(s_window_frame.origin.x + MINI_WINDOW_TITLE_PADDING_HORIZONTAL, s_window_frame.origin.y + MINI_WINDOW_PADDING_TOP, s_window_frame.size.w - (2 * MINI_WINDOW_TITLE_PADDING_HORIZONTAL), MINI_WINDOW_TITLE_HEIGHT);
    s_body_frame = GRect(s_window_frame.origin.x + MINI_WINDOW_BODY_MARGIN_HORIZONTAL, s_window_frame.origin.y + MINI_WINDOW_TITLE_HEIGHT + MINI_WINDOW_BODY_MARGIN_VERTICAL, s_window_frame.size.w - (2 * MINI_WINDOW_BODY_MARGIN_HORIZONTAL), s_window_frame.size.h - (MINI_WINDOW_TITLE_HEIGHT + (2 * MINI_WINDOW_BODY_MARGIN_VERTICAL)));

    // main layer
    s_main_layer = layer_create(s_window_frame);
    layer_set_update_proc(s_main_layer, main_layer_update_proc);

    // title layer
    s_title_layer = text_layer_create(s_title_frame);
    text_layer_set_background_color(s_title_layer, GColorClear);
    text_layer_set_text_color(s_title_layer, GColorWhite);
    text_layer_set_text(s_title_layer, title);
    text_layer_set_text_alignment(s_title_layer, GTextAlignmentLeft);

    // body text layer
    s_text_body_layer = text_layer_create(s_body_frame);
    text_layer_set_background_color(s_text_body_layer, GColorClear);
    text_layer_set_text(s_text_body_layer, body);
    text_layer_set_text_alignment(s_text_body_layer, GTextAlignmentLeft);

    s_is_created = true;
    
    show_mini_window();
}

void destroy_mini_window()
{
    if (s_is_created)
    {
        layer_destroy(s_main_layer);
        text_layer_destroy(s_title_layer);
        text_layer_destroy(s_text_body_layer);

        s_is_created = false;
    }
}

void show_mini_window()
{
    if (!s_is_showing)
    {
        layer_add_child(s_window_layer, s_main_layer);
        layer_add_child(s_window_layer, text_layer_get_layer(s_title_layer));
        layer_add_child(s_window_layer, text_layer_get_layer(s_text_body_layer)); //TODO later layer to scroll layer

        s_is_showing = true;
    }
}

void hide_mini_window()
{
    if (s_is_showing)
    {
        layer_remove_from_parent(s_main_layer);
        layer_remove_from_parent(text_layer_get_layer(s_title_layer));
        layer_remove_from_parent(text_layer_get_layer(s_text_body_layer)); //TODO later layer to scroll layer


        s_is_showing = false;
    }
}

bool is_showing_mini_window()
{
    return s_is_showing;
}