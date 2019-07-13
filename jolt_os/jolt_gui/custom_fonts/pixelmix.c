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
    0x45, 0x28, 0x8a, 0xc9, 0xa0,

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
    0xf8, 0x8f, 0x88, 0xf0,

    /* U+46 "F" */
    0xf8, 0x8f, 0x88, 0x80,

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
    0x88, 0x88, 0x88, 0xf0,

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
    0x78, 0x88, 0x70,

    /* U+64 "d" */
    0x8, 0x5b, 0x38, 0xc5, 0xe0,

    /* U+65 "e" */
    0x74, 0x7f, 0x7, 0x80,

    /* U+66 "f" */
    0x29, 0x2e, 0x92,

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
    0xf4, 0x63, 0xe8, 0x40,

    /* U+71 "q" */
    0x7c, 0x62, 0xf0, 0x84,

    /* U+72 "r" */
    0xb6, 0x61, 0x8, 0x0,

    /* U+73 "s" */
    0x7c, 0x1c, 0x1f, 0x0,

    /* U+74 "t" */
    0x44, 0xe4, 0x44, 0x30,

    /* U+75 "u" */
    0x8c, 0x63, 0x17, 0x0,

    /* U+76 "v" */
    0x8c, 0x62, 0xa2, 0x0,

    /* U+77 "w" */
    0x8c, 0x6b, 0x55, 0x0,

    /* U+78 "x" */
    0x8a, 0x88, 0xa8, 0x80,

    /* U+79 "y" */
    0x8c, 0x62, 0xf0, 0xb8,

    /* U+7A "z" */
    0xf8, 0x88, 0x8f, 0x80,

    /* U+7B "{" */
    0x69, 0x44, 0x98,

    /* U+7C "|" */
    0xfe,

    /* U+7D "}" */
    0xc9, 0x14, 0xb0,

    /* U+7E "~" */
    0x45, 0x44,

    /* U+7F "" */

    /* U+F000 "" */
    0xff, 0xf4, 0x0, 0xa0, 0x7, 0x0, 0x38, 0x1,
    0xc0, 0xb, 0xff, 0xc0,

    /* U+F001 "" */
    0xff, 0xf4, 0x0, 0xac, 0x7, 0x60, 0x3b, 0x1,
    0xc0, 0xb, 0xff, 0xc0,

    /* U+F002 "" */
    0xff, 0xf4, 0x0, 0xad, 0x87, 0x6c, 0x3b, 0x61,
    0xc0, 0xb, 0xff, 0xc0,

    /* U+F003 "" */
    0xff, 0xf4, 0x0, 0xad, 0xb7, 0x6d, 0xbb, 0x6d,
    0xc0, 0xb, 0xff, 0xc0,

    /* U+F004 "" */
    0x2, 0x7, 0xb7, 0xa3, 0x5, 0x1c, 0x39, 0xf9,
    0xc3, 0x8e, 0xc, 0x5e, 0xde, 0x4, 0x0,

    /* U+F005 "" */
    0xff, 0xf4, 0x0, 0xa6, 0x67, 0xc, 0x39, 0x99,
    0xc0, 0xb, 0xff, 0xc0,

    /* U+F006 "" */
    0x21, 0xaa, 0xe2, 0x3a, 0xa6, 0x20,

    /* U+F007 "" */
    0x8, 0x6, 0xa, 0x93, 0x9c, 0x9c, 0xe4, 0xa8,
    0x18, 0x8, 0x0,

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
    0x14, 0xa, 0xf, 0x9c, 0x72, 0x27, 0x1c, 0xf8,
    0x28, 0x14, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 32, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 32, .box_h = 7, .box_w = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 64, .box_h = 2, .box_w = 3, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 2, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 7, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 12, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 17, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 22, .adv_w = 32, .box_h = 2, .box_w = 1, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 23, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 26, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 29, .adv_w = 64, .box_h = 3, .box_w = 3, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 31, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 35, .adv_w = 48, .box_h = 2, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 36, .adv_w = 96, .box_h = 1, .box_w = 5, .ofs_x = 0, .ofs_y = 3},
    {.bitmap_index = 37, .adv_w = 48, .box_h = 2, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 41, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 54, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 59, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 79, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 89, .adv_w = 32, .box_h = 5, .box_w = 1, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 90, .adv_w = 48, .box_h = 6, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 92, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 96, .box_h = 3, .box_w = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 98, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 106, .adv_w = 128, .box_h = 7, .box_w = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 118, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 133, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 151, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 154, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 159, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 164, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 168, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 173, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 178, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 183, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 188, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 198, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 213, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 228, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 233, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 238, .adv_w = 48, .box_h = 7, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 240, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 243, .adv_w = 48, .box_h = 7, .box_w = 2, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 96, .box_h = 3, .box_w = 5, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 247, .adv_w = 80, .box_h = 1, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 248, .adv_w = 48, .box_h = 2, .box_w = 2, .ofs_x = 0, .ofs_y = 5},
    {.bitmap_index = 249, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 258, .adv_w = 80, .box_h = 5, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 270, .adv_w = 64, .box_h = 8, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 273, .adv_w = 96, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 277, .adv_w = 96, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 282, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 289, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 300, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 308, .adv_w = 96, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 312, .adv_w = 96, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 316, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 80, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 96, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 348, .adv_w = 96, .box_h = 5, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 352, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 32, .box_h = 7, .box_w = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 64, .box_h = 7, .box_w = 3, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 359, .adv_w = 96, .box_h = 3, .box_w = 5, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 361, .adv_w = 16, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 361, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 385, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 409, .adv_w = 208, .box_h = 9, .box_w = 13, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 424, .adv_w = 208, .box_h = 7, .box_w = 13, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 436, .adv_w = 80, .box_h = 9, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 442, .adv_w = 144, .box_h = 9, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 453, .adv_w = 96, .box_h = 3, .box_w = 3, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 455, .adv_w = 128, .box_h = 5, .box_w = 7, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 460, .adv_w = 144, .box_h = 7, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 468, .adv_w = 144, .box_h = 7, .box_w = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 476, .adv_w = 95, .box_h = 8, .box_w = 6, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 482, .adv_w = 144, .box_h = 9, .box_w = 9, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 96, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
        .glyph_id_start = 1, .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0
    },
    {
        .range_start = 61440, .range_length = 14, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
        .glyph_id_start = 97, .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0
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
    .cmap_num = 2,
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
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
};

#endif /*#if PIXELMIX*/

