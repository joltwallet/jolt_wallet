#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifndef DEJAVU_16
#define DEJAVU_16 1
#endif

#if DEJAVU_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+2B "+" */
    0xc, 0x3, 0x0, 0xc0, 0x30, 0xff, 0xff, 0xf0,
    0xc0, 0x30, 0xc, 0x3, 0x0,

    /* U+2C "," */
    0x77, 0x6e, 0xc0,

    /* U+2D "-" */
    0xff, 0xfc,

    /* U+2E "." */
    0xff, 0x80,

    /* U+2F "/" */
    0xc, 0x21, 0x86, 0x18, 0xc3, 0xc, 0x21, 0x86,
    0x10, 0xc2, 0x0,

    /* U+30 "0" */
    0x1c, 0xf, 0xc7, 0x3b, 0x8e, 0xe3, 0xb8, 0xfe,
    0x3f, 0x8f, 0xe3, 0x9c, 0xe7, 0xf0, 0xf8, 0x0,
    0x0,

    /* U+31 "1" */
    0x38, 0x7e, 0x3f, 0x3, 0x81, 0xc0, 0xe0, 0x70,
    0x38, 0x1c, 0xe, 0x3f, 0xdf, 0xe0,

    /* U+32 "2" */
    0x7c, 0x7f, 0xb3, 0xc0, 0xe0, 0x70, 0x38, 0x38,
    0x38, 0x78, 0x78, 0x3f, 0xff, 0xf0,

    /* U+33 "3" */
    0x7c, 0x7f, 0x81, 0xc0, 0xf0, 0x71, 0xf0, 0xfc,
    0xf, 0x3, 0x83, 0xff, 0xdf, 0xc0, 0x0,

    /* U+34 "4" */
    0x7, 0x3, 0xc1, 0xf0, 0x7c, 0x37, 0x19, 0xce,
    0x73, 0x1c, 0xff, 0xff, 0xf0, 0x70, 0x1c,

    /* U+35 "5" */
    0x7f, 0x7f, 0xbf, 0xdc, 0xf, 0xc7, 0xf9, 0x1c,
    0x7, 0x3, 0xc3, 0xff, 0xdf, 0xc0, 0x0,

    /* U+36 "6" */
    0xf, 0xf, 0xe7, 0x3, 0x80, 0xec, 0x3f, 0xcf,
    0x3b, 0x8e, 0xe3, 0x9c, 0xe7, 0xf8, 0xf8, 0x0,
    0x0,

    /* U+37 "7" */
    0xff, 0xff, 0xff, 0xe0, 0xe0, 0x70, 0x70, 0x38,
    0x38, 0x1c, 0x1c, 0xe, 0xf, 0x0,

    /* U+38 "8" */
    0x3e, 0x1f, 0xcf, 0x3b, 0x8e, 0x73, 0x8f, 0xc7,
    0xf3, 0x8e, 0xe3, 0xb8, 0xef, 0xf9, 0xfc, 0x0,
    0x0,

    /* U+39 "9" */
    0x1c, 0x1f, 0xcf, 0x7b, 0x8e, 0xe3, 0xb8, 0xe7,
    0xf8, 0xfe, 0x3, 0x81, 0xc7, 0xf1, 0xf8, 0x0,
    0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 160, .box_h = 10, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 58, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 16, .adv_w = 79, .box_h = 3, .box_w = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 18, .adv_w = 45, .box_h = 3, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 20, .adv_w = 94, .box_h = 14, .box_w = 6, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 31, .adv_w = 154, .box_h = 13, .box_w = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 48, .adv_w = 132, .box_h = 12, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 136, .box_h = 12, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 76, .adv_w = 141, .box_h = 13, .box_w = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 91, .adv_w = 155, .box_h = 12, .box_w = 10, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 141, .box_h = 13, .box_w = 9, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 121, .adv_w = 149, .box_h = 13, .box_w = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 138, .adv_w = 141, .box_h = 12, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 147, .box_h = 13, .box_w = 10, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 169, .adv_w = 149, .box_h = 13, .box_w = 10, .ofs_x = 0, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 43, .range_length = 15, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
        .glyph_id_start = 1, .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

/*Store all the custom data of the font*/
static lv_font_fmt_txt_dsc_t font_dsc = {
    .glyph_bitmap = gylph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .cmap_num = 1,
    .bpp = 1,

    .kern_scale = 0,
    .kern_dsc = NULL,
    .kern_classes = 0,
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
lv_font_t dejavu_16 = {
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .line_height = 12,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
};

#endif /*#if DEJAVU_16*/

