#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts:
 ******************************************************************************/

#ifndef MODENINE_20
    #define MODENINE_20 1
#endif

#if MODENINE_20

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
        /* U+2B "+" */
        0xc, 0x3, 0x0, 0xc0, 0x30, 0xff, 0xff, 0xf0, 0xc0, 0x30, 0xc, 0x3, 0x0,

        /* U+2C "," */
        0x33, 0x33, 0x7e, 0xc0,

        /* U+2D "-" */
        0xff, 0xff, 0xf0,

        /* U+2E "." */
        0xf0,

        /* U+2F "/" */
        0x0, 0x0, 0x20, 0x1c, 0xf, 0x7, 0x83, 0xc1, 0xe0, 0xf0, 0x78, 0x3c, 0xe, 0x1, 0x0,

        /* U+30 "0" */
        0x3f, 0x1f, 0xee, 0x1f, 0x3, 0xc0, 0xf0, 0x3c, 0xf, 0x3, 0xc0, 0xf0, 0x3c, 0xf, 0x87, 0x7f, 0x8f, 0xc0,

        /* U+31 "1" */
        0x31, 0xcf, 0x3c, 0x30, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0xff, 0xf0,

        /* U+32 "2" */
        0x3f, 0x1f, 0xee, 0x1f, 0x3, 0x0, 0xc0, 0x73, 0xf9, 0xfc, 0xe0, 0x30, 0xc, 0x3, 0x0, 0xff, 0xff, 0xf0,

        /* U+33 "3" */
        0xff, 0xff, 0xf0, 0xc, 0x7, 0x3, 0x81, 0xc0, 0xf0, 0x7e, 0x1, 0xc0, 0x3c, 0xf, 0x87, 0x7f, 0x8f, 0xc0,

        /* U+34 "4" */
        0x3, 0x1, 0xc0, 0xf0, 0x7c, 0x3b, 0x1c, 0xce, 0x33, 0xc, 0xff, 0xff, 0xf0, 0x30, 0xc, 0x3, 0x0, 0xc0,

        /* U+35 "5" */
        0xff, 0xff, 0xfc, 0x3, 0x0, 0xff, 0x3f, 0xe0, 0x1c, 0x3, 0x0, 0xc0, 0x3c, 0xf, 0x87, 0x7f, 0x8f, 0xc0,

        /* U+36 "6" */
        0xf, 0xf, 0xc7, 0x1, 0x80, 0xc0, 0x30, 0xf, 0xf3, 0xfe, 0xc1, 0xf0, 0x3c, 0xf, 0x87, 0x7f, 0x8f, 0xc0,

        /* U+37 "7" */
        0xff, 0xff, 0xf0, 0xc, 0x6, 0x3, 0x81, 0xc0, 0xc0, 0x60, 0x38, 0xc, 0x3, 0x0, 0xc0, 0x30, 0xc, 0x0,

        /* U+38 "8" */
        0x3f, 0x1f, 0xee, 0x1f, 0x3, 0xc0, 0xf8, 0x77, 0xf9, 0xfe, 0xe1, 0xf0, 0x3c, 0xf, 0x87, 0x7f, 0x8f, 0xc0,

        /* U+39 "9" */
        0x3f, 0x1f, 0xee, 0x1f, 0x3, 0xc0, 0xf8, 0x37, 0xfc, 0xff, 0x0, 0xc0, 0x30, 0x18, 0xe, 0x3f, 0xf, 0x0};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
        {.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
        {.bitmap_index = 0, .adv_w = 160, .box_h = 10, .box_w = 10, .ofs_x = 0, .ofs_y = 2},
        {.bitmap_index = 13, .adv_w = 64, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
        {.bitmap_index = 17, .adv_w = 160, .box_h = 2, .box_w = 10, .ofs_x = 0, .ofs_y = 6},
        {.bitmap_index = 20, .adv_w = 32, .box_h = 2, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 21, .adv_w = 160, .box_h = 12, .box_w = 10, .ofs_x = 0, .ofs_y = 1},
        {.bitmap_index = 36, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 54, .adv_w = 96, .box_h = 14, .box_w = 6, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 65, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 83, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 101, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 119, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 137, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 155, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 173, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
        {.bitmap_index = 191, .adv_w = 160, .box_h = 14, .box_w = 10, .ofs_x = 0, .ofs_y = 0}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] = {{.range_start       = 43,
                                                .range_length      = 15,
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
lv_font_t modenine_20 = {
        .dsc              = &font_dsc, /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
        .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
        .get_glyph_dsc    = lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
        .line_height      = 14,                            /*The maximum line height required by the font*/
        .base_line        = 0,                             /*Baseline measured from the bottom of the line*/
};

#endif /*#if MODENINE_20*/
