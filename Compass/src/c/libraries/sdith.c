// Created by Stefan Bauwens 2026
// Small Dithering Library
#include "sdith.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define CLAMP(x, min, max) (MAX(min, MIN(x, max)))

DitherType _dither_type = DITHER_FILLED;
GColor _light_color = GColorWhite;
GColor _dark_color = GColorBlack;
GColor _pixel_color = GColorRichBrilliantLavender;

/* CORNERS START */
// Taken from graphics.c from PebbleOS, logic is looseley inspired on the code from graphics.c
static const uint32_t round_top_corner_lookup[] = {
    0x0, 0x01, 0x01, 0x12, 0x113, 0x123, 0x1234, 0x11235, 0x112346,
};
static const uint32_t round_bottom_corner_lookup[] = {
    0x0, 0x01, 0x10, 0x210, 0x3110, 0x32100, 0x432100, 0x5321100, 0x64321100,
};

/* CORNERS END */

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
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_10 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        1, 0, 1, 1,
        0, 1, 0, 1,
        1, 1, 1, 0
    },
    .width = 4,
    .height = 4
};

static const DitherPattern DITHER_PATTERN_11 = {
    .pattern = (const bool[]) {
        0, 1, 0, 1,
        1, 1, 1, 1,
        0, 1, 0, 1,
        1, 1, 1, 0
    },
    .width = 4,
    .height = 4
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

DitherTemplate sdith_color_to_dither_template(GColor color)
{
    DitherTemplate color_template = {
        .dither_type = DITHER_FILLED,
        .light_color = color,
        .dark_color = color
    };
    return color_template;
}

void sdith_set_dither_type(DitherType dither_type)
{
    _dither_type = dither_type;
}

void sdith_set_fill_colors(GColor light_color, GColor dark_color)
{
    _light_color = light_color;
    _dark_color = dark_color;
}

void sdith_set_dither_template(DitherTemplate dither_template)
{
    sdith_set_dither_type(dither_template.dither_type);
    sdith_set_fill_colors(dither_template.light_color, dither_template.dark_color);
}

void _set_pixel_color(GContext * ctx, GColor color)
{
    //if (!gcolor_equal(_pixel_color, color))
    //{
        _pixel_color = color;
        graphics_context_set_stroke_color(ctx, color);
    //}
}

void sdith_draw_pixel(GContext * ctx, GPoint point)
{
    GColor color;
    if (gcolor_equal(_dark_color, _light_color)) // shortcut for just using regular fill
    {
        color = _dark_color;
    }
    else
    {
        DitherPattern pattern = Patterns[_dither_type];
        int16_t modx = point.x % pattern.width;
        int16_t mody = point.y % pattern.height;
        //APP_LOG(APP_LOG_LEVEL_DEBUG, "modx: %d mody: %d", (int)modx, (int)mody);
        color = pattern.pattern[mody * pattern.width + modx] == 1 ? _dark_color : _light_color;
    }

    _set_pixel_color(ctx, color);
    if (!gcolor_equal(color, GColorClear))
    {
        graphics_draw_pixel(ctx, point);
    }
}

void _optimzed_draw_pixel(GContext * ctx, GPoint point, DitherPattern pattern)
{
    int16_t modx = point.x % pattern.width;
    int16_t mody = point.y % pattern.height;
    GColor color = pattern.pattern[mody * pattern.width + modx] == 1 ? _dark_color : _light_color;

    _set_pixel_color(ctx, color);
    if (!gcolor_equal(color, GColorClear))
    {
        graphics_draw_pixel(ctx, point);
    }
}

void _draw_horizontal_line(GContext * ctx, GPoint point, int16_t length, DitherPattern pattern)
{
    for (int16_t i = 0; i < length; i++)
    {
        _optimzed_draw_pixel(ctx, GPoint(point.x + i, point.y), pattern);
    }
}

void _draw_corner(GContext * ctx, GPoint point, int8_t radius, bool is_left_corner, uint32_t corner_insets, DitherPattern pattern) // radius MUST be between 0 and 8
{
    // not doing any clamping of radius since this is an internal function so *should* be safe

    int8_t row_inset = 0;
    int8_t length = 0;
    for (int8_t row = 0; row < radius; row++)
    {
        row_inset = (corner_insets >> (4 * row)) & 0xF; // reading from most right to left
        length = radius - row_inset;
        if (is_left_corner)
        {
            _draw_horizontal_line(ctx, GPoint(point.x + row_inset, point.y + row), length, pattern);
        }
        else // right corner
        {
            _draw_horizontal_line(ctx, GPoint(point.x, point.y + row), length, pattern);
        }
    }
}

void _fill_rect(GContext * ctx, GRect rect, DitherPattern pattern)
{
    for (int16_t h = 0; h < rect.size.h; h++)
    {
        for (int16_t w = 0; w < rect.size.w; w++) 
        {
            _optimzed_draw_pixel(ctx, GPoint(rect.origin.x + w, rect.origin.y + h), pattern);
        }
    }
}

void sdith_fill_rect(GContext * ctx, GRect rect, uint16_t corner_radius, GCornerMask corner_mask)
{
    if (gcolor_equal(_dark_color, _light_color)) // shortcut for just using regular fill
    {
        GColor color = _dark_color;
        graphics_context_set_fill_color(ctx, color);
        graphics_fill_rect(ctx, rect, corner_radius, corner_mask);
    }
    else   
    {
        DitherPattern pattern = Patterns[_dither_type];

        if (corner_mask == GCornerNone || corner_radius == 0)
        {
            _fill_rect(ctx, rect, pattern);
            return;
        }

        // clamp corner_radius to 8 or to smallest width/height of rect
        corner_radius = CLAMP(corner_radius, 0, MIN(MIN(8, rect.size.h), rect.size.w));

        //  use simple rect fill if corner radius ended up being 0 after clamp
        if (corner_radius == 0)
        {
            _fill_rect(ctx, rect, pattern);
            return;
        }

        // using corners
        bool has_top_left_corner = corner_mask & GCornerTopLeft;
        bool has_top_right_corner = corner_mask & GCornerTopRight;
        bool has_bottom_left_corner = corner_mask & GCornerBottomLeft;
        bool has_bottom_right_corner = corner_mask & GCornerBottomRight;

        uint32_t corner_insets;
        GRect inner_rect;

        if (has_top_left_corner || has_top_right_corner)
        {
            // Set up the insets for doing the top corners
            corner_insets = round_top_corner_lookup[corner_radius];
            
            inner_rect = GRect(
                (has_top_left_corner ? corner_radius : 0) + rect.origin.x, 
                rect.origin.y, 
                (rect.size.w - (has_top_left_corner ? corner_radius : 0)) - (has_top_right_corner ? corner_radius : 0),
                corner_radius
            );

            //draw rect between top corners
            _fill_rect(ctx, inner_rect, pattern);

            if (has_top_left_corner)
            {
                //draw top left corner
                _draw_corner(ctx, rect.origin, corner_radius, true, corner_insets, pattern);
            }
            if (has_top_right_corner)
            {
                //draw top right corner
                _draw_corner(ctx, GPoint(inner_rect.origin.x + inner_rect.size.w, rect.origin.y), corner_radius, false, corner_insets, pattern);
            }
        }

        // draw middle rect
        GRect middle_rect = GRect(
            rect.origin.x,
            rect.origin.y + ((has_top_left_corner || has_top_right_corner) ? corner_radius : 0 ), 
            rect.size.w, 
            MAX(rect.size.h - ((has_top_left_corner || has_top_right_corner) ? corner_radius : 0 ) - ((has_bottom_left_corner || has_bottom_right_corner) ? corner_radius : 0), 0)
        );
        _fill_rect(ctx, middle_rect, pattern);

        if (has_bottom_left_corner || has_bottom_right_corner)
        {
            // Set up the insets for doing the bottom corners
            corner_insets = round_bottom_corner_lookup[corner_radius];

            inner_rect = GRect(
                (has_bottom_left_corner ? corner_radius : 0) + rect.origin.x, 
                middle_rect.origin.y + middle_rect.size.h, 
                (rect.size.w - (has_bottom_left_corner ? corner_radius : 0)) - (has_bottom_right_corner ? corner_radius : 0),
                corner_radius
            );

            //draw rect between top corners
            _fill_rect(ctx, inner_rect, pattern);

            if (has_bottom_left_corner)
            {
                //draw top left corner
                _draw_corner(ctx, GPoint(rect.origin.x, inner_rect.origin.y), corner_radius, true, corner_insets, pattern);
            }
            if (has_bottom_right_corner)
            {
                //draw top right corner
                _draw_corner(ctx, GPoint(inner_rect.origin.x + inner_rect.size.w, inner_rect.origin.y), corner_radius, false, corner_insets, pattern);
            }
        }
    }
}

