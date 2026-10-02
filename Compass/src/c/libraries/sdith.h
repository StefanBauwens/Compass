// Created by Stefan Bauwens 2026
// Simple Monochrome Dithering Library
#ifndef SDITH_H

#define SDITH_H
#define COLOR_TEMPLATE(color) \
{ \
    .dither_type = DITHER_FILLED, \
    .light_color = (color), \
    .dark_color = (color) \
}

#define DITHER_TEMPLATE(type, light, dark) \
{ \
    .dither_type = (type), \
    .light_color = (light), \
    .dark_color = (dark) \
}

#include <pebble.h>

typedef enum {
    DITHER_BLANK = 0, 
    DITHER_1,
    DITHER_2,
    DITHER_3,
    DITHER_4,
    DITHER_5,
    DITHER_6,
    DITHER_7,
    DITHER_8, // checkered
    DITHER_9,
    DITHER_10,
    DITHER_11,
    DITHER_12,
    DITHER_13,
    DITHER_14,
    DITHER_15,
    DITHER_FILLED, // filled
} DitherType;

typedef struct {
    const bool *pattern;
    uint8_t width;
    uint8_t height;
} DitherPattern;

typedef struct {
    DitherType dither_type;
    GColor light_color;
    GColor dark_color;
} DitherTemplate;

DitherTemplate sdith_color_to_dither_template(GColor color);
void sdith_set_dither_type(DitherType dither_type);
void sdith_set_fill_colors(GColor light_color, GColor dark_color);
void sdith_set_dither_template(DitherTemplate dither_template);
void sdith_draw_pixel(GContext * ctx, GPoint point);
void sdith_fill_rect(GContext * ctx, GRect rect, uint16_t corner_radius, GCornerMask corner_mask);

#endif // SDITH_H