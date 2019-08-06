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
    0x18, 0x18, 0x18, 0xff, 0xff, 0x18, 0x18, 0x18,

    /* U+2C "," */
    0x6d, 0xbc,

    /* U+2D "-" */
    0xff, 0xc0,

    /* U+2E "." */
    0xfc,

    /* U+2F "/" */
    0x3, 0x6, 0x6, 0xc, 0xc, 0x1c, 0x18, 0x38,
    0x30, 0x30, 0x60, 0x60, 0xc0,

    /* U+30 "0" */
    0x3c, 0x7e, 0x66, 0xc3, 0xc3, 0xdb, 0xdb, 0xc3,
    0xc3, 0x66, 0x7e, 0x3c,

    /* U+31 "1" */
    0x38, 0x78, 0x58, 0x18, 0x18, 0x18, 0x18, 0x18,
    0x18, 0x18, 0xff, 0xff,

    /* U+32 "2" */
    0x7c, 0xfe, 0x87, 0x3, 0x3, 0x6, 0xc, 0x18,
    0x30, 0x60, 0xff, 0xff,

    /* U+33 "3" */
    0x7c, 0xfe, 0x87, 0x3, 0x7, 0x3e, 0x3e, 0x7,
    0x3, 0x83, 0xfe, 0x7c,

    /* U+34 "4" */
    0xe, 0x1e, 0x1e, 0x36, 0x66, 0x66, 0xc6, 0xff,
    0xff, 0x6, 0x6, 0x6,

    /* U+35 "5" */
    0xfe, 0xfe, 0xc0, 0xc0, 0xfc, 0xfe, 0x87, 0x3,
    0x3, 0x87, 0xfe, 0x7c,

    /* U+36 "6" */
    0x3c, 0x7e, 0x62, 0xc0, 0xdc, 0xfe, 0xe7, 0xc3,
    0xc3, 0x67, 0x7e, 0x3c,

    /* U+37 "7" */
    0xff, 0xff, 0x7, 0x6, 0xe, 0xc, 0xc, 0x18,
    0x18, 0x30, 0x30, 0x60,

    /* U+38 "8" */
    0x3c, 0x7e, 0xe7, 0xc3, 0xc3, 0x7e, 0x7e, 0xe7,
    0xc3, 0xe7, 0x7e, 0x3c,

    /* U+39 "9" */
    0x3c, 0x7e, 0xe6, 0xc3, 0xc3, 0xe7, 0x7f, 0x3b,
    0x3, 0x46, 0x7e, 0x3c
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 154, .box_h = 8, .box_w = 8, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 8, .adv_w = 154, .box_h = 5, .box_w = 3, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 10, .adv_w = 154, .box_h = 2, .box_w = 5, .ofs_x = 2, .ofs_y = 3},
    {.bitmap_index = 12, .adv_w = 154, .box_h = 3, .box_w = 2, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 154, .box_h = 13, .box_w = 8, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 26, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 86, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 98, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 110, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 122, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 154, .box_h = 12, .box_w = 8, .ofs_x = 1, .ofs_y = 0}
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

