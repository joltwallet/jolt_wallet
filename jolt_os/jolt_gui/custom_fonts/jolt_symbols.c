#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 8 px
 * Bpp: 1
 * Opts:
 ******************************************************************************/

#ifndef JOLT_SYMBOLS
    #define JOLT_SYMBOLS 1
#endif

#if JOLT_SYMBOLS

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
        /* U+F000 "" */
        0xff, 0xf4, 0x0, 0xa0, 0x7, 0x0, 0x38, 0x1, 0xc0, 0xb, 0xff, 0xc0,

        /* U+F001 "" */
        0xff, 0xf4, 0x0, 0xac, 0x7, 0x60, 0x3b, 0x1, 0xc0, 0xb, 0xff, 0xc0,

        /* U+F002 "" */
        0xff, 0xf4, 0x0, 0xad, 0x87, 0x6c, 0x3b, 0x61, 0xc0, 0xb, 0xff, 0xc0,

        /* U+F003 "" */
        0xff, 0xf4, 0x0, 0xad, 0xb7, 0x6d, 0xbb, 0x6d, 0xc0, 0xb, 0xff, 0xc0,

        /* U+F004 "" */
        0x2, 0x7, 0xb7, 0xa3, 0x5, 0x1c, 0x39, 0xf9, 0xc3, 0x8e, 0xc, 0x5e, 0xde, 0x4, 0x0,

        /* U+F005 "" */
        0xff, 0xf4, 0x0, 0xa6, 0x67, 0xc, 0x39, 0x99, 0xc0, 0xb, 0xff, 0xc0,

        /* U+F006 "" */
        0x21, 0xaa, 0xe2, 0x3a, 0xa6, 0x20,

        /* U+F007 "" */
        0x8, 0x6, 0xa, 0x93, 0x9c, 0x9c, 0xe4, 0xa8, 0x18, 0x8, 0x0,

        /* U+F008 "" */
        0x5d, 0x0,

        /* U+F009 "" */
        0x38, 0x8a, 0x49, 0xc1, 0x0,

        /* U+F00A "" */
        0x3e, 0x20, 0xa7, 0x24, 0x44, 0x90, 0xe0, 0x20,

        /* U+F00B "" */
        0x3e, 0x20, 0xa0, 0x28, 0x22, 0x20, 0xa0, 0x20,

        /* U+F00C "" */
        0x31, 0x24, 0xbf, 0xff, 0xff, 0xff,

        /* U+F00D "" */
        0x14, 0xa, 0xf, 0x9c, 0x72, 0x27, 0x1c, 0xf8, 0x28, 0x14, 0x0};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
        {.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
        {.bitmap_index = 0, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 12, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 24, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 36, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 48, .adv_w = 208, .box_h = 9, .box_w = 13, .ofs_x = 0, .ofs_y = -1},
        {.bitmap_index = 63, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 75, .adv_w = 80, .box_h = 9, .box_w = 5, .ofs_x = 0, .ofs_y = -1},
        {.bitmap_index = 81, .adv_w = 144, .box_h = 9, .box_w = 9, .ofs_x = 0, .ofs_y = -1},
        {.bitmap_index = 92, .adv_w = 96, .box_h = 3, .box_w = 3, .ofs_x = 3, .ofs_y = 0},
        {.bitmap_index = 94, .adv_w = 128, .box_h = 5, .box_w = 7, .ofs_x = 1, .ofs_y = 0},
        {.bitmap_index = 99, .adv_w = 144, .box_h = 7, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 107, .adv_w = 144, .box_h = 7, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 115, .adv_w = 95, .box_h = 8, .box_w = 6, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 121, .adv_w = 144, .box_h = 9, .box_w = 9, .ofs_x = 0, .ofs_y = 0}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] = {{.range_start       = 61440,
                                                .range_length      = 14,
                                                .type              = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
                                                .glyph_id_start    = 1,
                                                .unicode_list      = NULL,
                                                .glyph_id_ofs_list = NULL,
                                                .list_length       = 0}};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
        .glyph_bitmap = gylph_bitmap,
        .glyph_dsc    = glyph_dsc,
        .cmaps        = cmaps,
        .cmap_num     = 1,
        .bpp          = 1,

        .kern_scale   = 0,
        .kern_dsc     = NULL,
        .kern_classes = 0,
};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t jolt_symbols = {
        .dsc              = &font_dsc, /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
        .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
        .get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
        .line_height      = 9,                             /*The maximum line height required by the font*/
        .base_line        = 1,                             /*Baseline measured from the bottom of the line*/
};

#endif /*#if JOLT_SYMBOLS*/
