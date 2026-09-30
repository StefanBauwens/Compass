#define MINI_WINDOW_WIDTH PBL_DISPLAY_WIDTH - 40
#define MINI_WINDOW_HEIGHT PBL_DISPLAY_HEIGHT - 80
#define MINI_WINDOW_X 20
#define MINI_WINDOW_Y 20
#define MINI_WINDOW_TITLE_HEIGHT 20
#define MINI_WINDOW_TITLE_WIDTH MINI_WINDOW_WIDTH - 10
#define MINI_WINDOW_TITLE_X MINI_WINDOW_X + 5
#define MINI_WINDOW_TITLE_Y MINI_WINDOW_X + 1


#include "miniwindow.h"
#include "sdith.h"

static TextLayer *s_title_layer;
static TextLayer *s_text_body_layer;
static Layer *s_main_layer;
static Layer *s_window_layer;
static bool s_is_created = false;
static bool s_is_showing = false;
static const GRect s_window_frame = GRect(MINI_WINDOW_X, MINI_WINDOW_Y, MINI_WINDOW_WIDTH, MINI_WINDOW_HEIGHT);
static const GRect s_title_frame = GRect(MINI_WINDOW_TITLE_X, MINI_WINDOW_TITLE_Y, MINI_WINDOW_TITLE_WIDTH, MINI_WINDOW_TITLE_HEIGHT);
static const GRect s_body_frame = GRect(MINI_WINDOW_X + 3, MINI_WINDOW_Y + 22, MINI_WINDOW_WIDTH - 6 , MINI_WINDOW_HEIGHT - 25); //TODO test

static const DitherTemplate bg_dith = DITHER_TEMPLATE(DITHER_8, GColorClear, GColorWhite);

void main_layer_update_proc(Layer *layer, GContext *ctx) { 
    graphics_context_set_fill_color(ctx, GColorBlack);
    //graphics_fill_rect(ctx, GRect(0, 0, s_window_frame.size.w, s_window_frame.size.h), 4, GCornersAll);
    graphics_fill_rect(ctx, GRect(0, 0, s_window_frame.size.w, MINI_WINDOW_TITLE_HEIGHT), 4, GCornersTop); //TODO temp

    sdith_set_dither_template(bg_dith);
    sdith_fill_rect(ctx, GRect(2, 2 + MINI_WINDOW_TITLE_HEIGHT, s_window_frame.size.w-4, s_window_frame.size.h-4- MINI_WINDOW_TITLE_HEIGHT));

    //TODO temp commented
    //graphics_context_set_fill_color(ctx, GColorWhite);
    //graphics_fill_rect(ctx, GRect(2, 2 + MINI_WINDOW_TITLE_HEIGHT, s_window_frame.size.w-4, s_window_frame.size.h-4- MINI_WINDOW_TITLE_HEIGHT), 2, GCornersBottom);
}

void create_mini_window(Layer * window_layer, const char * title, const char * body)
{
    if (s_is_created)
    {
        return;
    }

    s_window_layer = window_layer;

    // main layer
    s_main_layer = layer_create(s_window_frame);
    layer_set_update_proc(s_main_layer, main_layer_update_proc);

    // title layer
    s_title_layer = text_layer_create(s_title_frame);
    text_layer_set_background_color(s_title_layer, GColorBlack);
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