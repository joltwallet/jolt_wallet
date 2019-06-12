#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 8 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifndef PIXELMIX
#define PIXELMIX 1
#endif

#if PIXELMIX

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */

    /* U+21 "!" */
    0xfa,

    /* U+22 "\"" */
    0xb4,

    /* U+23 "#" */
    0x57, 0xd4, 0xa5, 0x7d, 0x40,

    /* U+24 "$" */
    0x75, 0x68, 0xe2, 0xd5, 0xc0,

    /* U+25 "%" */
    0xce, 0x84, 0x44, 0x2e, 0x60,

    /* U+26 "&" */
    0xe2, 0x8a, 0x10, 0xb6, 0x27, 0x40,

    /* U+27 "'" */
    0xc0,

    /* U+28 "(" */
    0x2a, 0x48, 0x88,

    /* U+29 ")" */
    0x88, 0x92, 0xa0,

    /* U+2A "*" */
    0x5d, 0x0,

    /* U+2B "+" */
    0x21, 0x3e, 0x42, 0x0,

    /* U+2C "," */
    0xd0,

    /* U+2D "-" */
    0xf8,

    /* U+2E "." */
    0xf0,

    /* U+2F "/" */
    0x25, 0x25, 0x20,

    /* U+30 "0" */
    0x74, 0x67, 0x5c, 0xc5, 0xc0,

    /* U+31 "1" */
    0x59, 0x24, 0xb8,

    /* U+32 "2" */
    0x74, 0x42, 0x22, 0x23, 0xe0,

    /* U+33 "3" */
    0xf8, 0x88, 0x20, 0xc5, 0xc0,

    /* U+34 "4" */
    0x11, 0x95, 0x2f, 0x88, 0x40,

    /* U+35 "5" */
    0xfc, 0x3c, 0x10, 0xc5, 0xc0,

    /* U+36 "6" */
    0x32, 0x21, 0xe8, 0xc5, 0xc0,

    /* U+37 "7" */
    0xf8, 0x44, 0x44, 0x21, 0x0,

    /* U+38 "8" */
    0x74, 0x62, 0xe8, 0xc5, 0xc0,

    /* U+39 "9" */
    0x74, 0x62, 0xf0, 0x89, 0x80,

    /* U+3A ":" */
    0x88,

    /* U+3B ";" */
    0x40, 0xd0,

    /* U+3C "<" */
    0x12, 0x48, 0x42, 0x10,

    /* U+3D "=" */
    0xf8, 0x3e,

    /* U+3E ">" */
    0x84, 0x21, 0x24, 0x80,

    /* U+3F "?" */
    0x69, 0x12, 0x40, 0x40,

    /* U+40 "@" */
    0x7d, 0x6, 0xed, 0x5b, 0xf0, 0x1f, 0x0,

    /* U+41 "A" */
    0x74, 0x63, 0xf8, 0xc6, 0x20,

    /* U+42 "B" */
    0xf4, 0x63, 0xe8, 0xc7, 0xc0,

    /* U+43 "C" */
    0x74, 0x61, 0x8, 0x45, 0xc0,

    /* U+44 "D" */
    0xf4, 0x63, 0x18, 0xc7, 0xc0,

    /* U+45 "E" */
    0xfc, 0x21, 0xe8, 0x43, 0xe0,

    /* U+46 "F" */
    0xfc, 0x21, 0xe8, 0x42, 0x0,

    /* U+47 "G" */
    0x74, 0x61, 0x78, 0xc5, 0xe0,

    /* U+48 "H" */
    0x8c, 0x63, 0xf8, 0xc6, 0x20,

    /* U+49 "I" */
    0xe9, 0x24, 0xb8,

    /* U+4A "J" */
    0x38, 0x84, 0x21, 0x49, 0x80,

    /* U+4B "K" */
    0x8c, 0xa9, 0x8a, 0x4a, 0x20,

    /* U+4C "L" */
    0x84, 0x21, 0x8, 0x43, 0xe0,

    /* U+4D "M" */
    0x8e, 0xeb, 0x58, 0xc6, 0x20,

    /* U+4E "N" */
    0x8c, 0x73, 0x59, 0xc6, 0x20,

    /* U+4F "O" */
    0x74, 0x63, 0x18, 0xc5, 0xc0,

    /* U+50 "P" */
    0xf4, 0x63, 0xe8, 0x42, 0x0,

    /* U+51 "Q" */
    0x74, 0x63, 0x1a, 0xc9, 0xa0,

    /* U+52 "R" */
    0xf4, 0x63, 0xea, 0x4a, 0x20,

    /* U+53 "S" */
    0x7c, 0x20, 0xe0, 0x87, 0xc0,

    /* U+54 "T" */
    0xf9, 0x8, 0x42, 0x10, 0x80,

    /* U+55 "U" */
    0x8c, 0x63, 0x18, 0xc5, 0xc0,

    /* U+56 "V" */
    0x8c, 0x63, 0x15, 0x28, 0x80,

    /* U+57 "W" */
    0x8c, 0x63, 0x5a, 0xd5, 0x40,

    /* U+58 "X" */
    0x8c, 0x54, 0x45, 0x46, 0x20,

    /* U+59 "Y" */
    0x8c, 0x62, 0xa2, 0x10, 0x80,

    /* U+5A "Z" */
    0xf8, 0x44, 0x44, 0x43, 0xe0,

    /* U+5B "[" */
    0xea, 0xac,

    /* U+5C "\\" */
    0x91, 0x24, 0x48,

    /* U+5D "]" */
    0xd5, 0x5c,

    /* U+5E "^" */
    0x22, 0xa2,

    /* U+5F "_" */
    0xf0,

    /* U+60 "`" */
    0x90,

    /* U+61 "a" */
    0x70, 0x5f, 0x17, 0x80,

    /* U+62 "b" */
    0x84, 0x2d, 0x98, 0xc7, 0xc0,

    /* U+63 "c" */
    0x74, 0x21, 0x17, 0x0,

    /* U+64 "d" */
    0x8, 0x5b, 0x38, 0xc5, 0xe0,

    /* U+65 "e" */
    0x74, 0x7f, 0x7, 0x0,

    /* U+66 "f" */
    0x32, 0x50, 0x8e, 0x21, 0x8,

    /* U+67 "g" */
    0x7c, 0x62, 0xf0, 0xb8,

    /* U+68 "h" */
    0x84, 0x2d, 0x98, 0xc6, 0x20,

    /* U+69 "i" */
    0x43, 0x24, 0xb8,

    /* U+6A "j" */
    0x10, 0x11, 0x19, 0x60,

    /* U+6B "k" */
    0x88, 0x9a, 0xca, 0x90,

    /* U+6C "l" */
    0xc9, 0x24, 0xb8,

    /* U+6D "m" */
    0xd5, 0x6b, 0x18, 0x80,

    /* U+6E "n" */
    0xb6, 0x63, 0x18, 0x80,

    /* U+6F "o" */
    0x74, 0x63, 0x17, 0x0,

    /* U+70 "p" */
    0xf4, 0x7d, 0x8, 0x0,

    /* U+71 "q" */
    0x6c, 0xde, 0x10, 0x80,

    /* U+72 "r" */
    0xb6, 0x61, 0x8, 0x0,

    /* U+73 "s" */
    0x74, 0x1c, 0x1f, 0x0,

    /* U+74 "t" */
    0x42, 0x38, 0x84, 0x24, 0xc0,

    /* U+75 "u" */
    0x8c, 0x63, 0x17, 0x0,

    /* U+76 "v" */
    0x8c, 0x62, 0xa2, 0x0,

    /* U+77 "w" */
    0x8c, 0x6b, 0x55, 0x0,

    /* U+78 "x" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+79 "y" */
    0x8c, 0x5e, 0x17, 0x0,

    /* U+7A "z" */
    0xf8, 0x88, 0x8f, 0x80,

    /* U+7B "{" */
    0x69, 0x44, 0x98,

    /* U+7C "|" */
    0xfe,

    /* U+7D "}" */
    0xc9, 0x14, 0xb0,

    /* U+7E "~" */
    0x45, 0x44
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 80, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 32, .box_h = 7, .box_w = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 64, .box_h = 2, .box_w = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 2, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 112, .box_h = 7, .box_w = 6, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 23, .adv_w = 32, .box_h = 2, .box_w = 1, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 24, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 27, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 30, .adv_w = 64, .box_h = 3, .box_w = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 32, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 36, .adv_w = 48, .box_h = 2, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 96, .box_h = 1, .box_w = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 38, .adv_w = 48, .box_h = 2, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 42, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 47, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 60, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 65, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 70, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 80, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 85, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 32, .box_h = 5, .box_w = 1, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 91, .adv_w = 48, .box_h = 6, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 96, .box_h = 3, .box_w = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 99, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 107, .adv_w = 128, .box_h = 7, .box_w = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 119, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 124, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 129, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 144, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 157, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 162, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 167, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 177, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 182, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 187, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 197, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 207, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 212, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 217, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 222, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 232, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 48, .box_h = 7, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 247, .adv_w = 48, .box_h = 7, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 249, .adv_w = 96, .box_h = 3, .box_w = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 251, .adv_w = 80, .box_h = 1, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 252, .adv_w = 48, .box_h = 2, .box_w = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 253, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 262, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 271, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 96, .box_h = 8, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 96, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 284, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 289, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 292, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 303, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 307, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 311, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 323, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 327, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 331, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 363, .adv_w = 32, .box_h = 7, .box_w = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 367, .adv_w = 96, .box_h = 3, .box_w = 5, .ofs_x = 0, .ofs_y = 2}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
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
lv_font_t pixelmix = {
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .line_height = 8,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
};

#endif /*#if PIXELMIX*/

