// Created by Stefan Bauwens 2026
// Small Dithering Library
#include "sdith.h"

DitherType _dither_type = DITHER_FILLED;
GColor _light_color = GColorWhite;
GColor _dark_color = GColorBlack;

/* DITHER PATTERNS START */
static const DitherPattern DITHER_PATTERN_0 = {
    .pattern = (const bool[]) {
        0
    },
    .width = 1,
    .height = 1
};

static const DitherPattern DITHER_PATTERN_1 = {
    .pattern = (const bool[]) {
        1, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_2 = {
    .pattern = (const bool[]) {
        1, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 0
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_3 = {
    .pattern = (const bool[]) {
        1, 0, 1, 0,
        0, 0, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 0
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_4 = {
    .pattern = (const bool[]) {
        1, 0,
        0, 0
    },
    .width = 2,
    .height = 2
};

static const DitherPattern DITHER_PATTERN_5 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        0, 0, 0, 0,
        0, 1, 0, 1,
        1, 0, 0, 0
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_6 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        0, 0, 1, 0,
        0, 1, 0, 1,
        1, 0, 0, 0
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_7 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        0, 0, 1, 0,
        0, 1, 0, 1,
        1, 0, 1, 0
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_8 = {
    .pattern = (const bool[]) {
        0, 1,
        1, 0
    },
    .width = 2,
    .height = 2
};

static const DitherPattern DITHER_PATTERN_9 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        1, 0, 1, 1,
        0, 1, 0, 1,
        1, 0, 1, 0
    },
    .width = 2,
    .height = 2
};

static const DitherPattern DITHER_PATTERN_10 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        1, 0, 1, 1,
        0, 1, 0, 1,
        1, 1, 1, 0
    },
    .width = 2,
    .height = 2
};

static const DitherPattern DITHER_PATTERN_11 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        1, 1, 1, 1,
        0, 1, 0, 1,
        1, 1, 1, 0
    },
    .width = 2,
    .height = 2
};

static const DitherPattern DITHER_PATTERN_12 = {
    .pattern = (const bool[]) {
        0, 1,
        1, 1
    },
    .width = 2,
    .height = 2
};

static const DitherPattern DITHER_PATTERN_13 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        1, 1, 1, 1,
        1, 1, 0, 1,
        1, 1, 1, 1
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_14 = {
    .pattern = (const bool[]) {
        0, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 0, 1,
        1, 1, 1, 1
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_15 = {
    .pattern = (const bool[]) {
        0, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_16 = {
    .pattern = (const bool[]) {
        1
    },
    .width = 1,
    .height = 1
};

/* DITHER PATTERNS END*/

static const DitherPattern Patterns[] = { // should match the enum order
    DITHER_PATTERN_0, // blank
    DITHER_PATTERN_1,
    DITHER_PATTERN_2,
    DITHER_PATTERN_3,
    DITHER_PATTERN_4,
    DITHER_PATTERN_5,
    DITHER_PATTERN_6,
    DITHER_PATTERN_7,
    DITHER_PATTERN_8, // checkered
    DITHER_PATTERN_9,
    DITHER_PATTERN_10,
    DITHER_PATTERN_11,
    DITHER_PATTERN_12,
    DITHER_PATTERN_13,
    DITHER_PATTERN_14,
    DITHER_PATTERN_15,
    DITHER_PATTERN_16, // filled
};

void sdith_set_dither_type(DitherType dither_type)
{
    _dither_type = dither_type;
}

void sdith_set_fill_colors(GColor light_color, GColor dark_color)
{
    _light_color = light_color;
    _dark_color = dark_color;
}

void sdith_draw_pixel(GContext * ctx, GPoint point)
{
    DitherPattern pattern = Patterns[_dither_type];
    int16_t modx = point.x % pattern.width;
    int16_t mody = point.y % pattern.height;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "modx: %d mody: %d", (int)modx, (int)mody);
    GColor color = pattern.pattern[mody * pattern.width + modx] == 1 ? _light_color : _dark_color;
    graphics_context_set_stroke_color(ctx, color);
    graphics_draw_pixel(ctx, point);
}

void sdith_fill_rect(GContext * ctx, GRect rect)
{
    for (int16_t h = 0; h < rect.size.h; h++)
    {
        for (int16_t w = 0; w < rect.size.w; w++) 
        {
            sdith_draw_pixel(ctx, GPoint(rect.origin.x + w, rect.origin.y + h));
        }
    }
}


