#include "lvgl/lvgl.h"

/*******************************************************************************
 * Size: 9 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifndef TERMINUS_9
#define TERMINUS_9 1
#endif

#if TERMINUS_9

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t gylph_bitmap[] = {
    /* U+20 " " */

    /* U+21 "!" */
    0x60,

    /* U+22 "\"" */
    0x14, 0x0,

    /* U+23 "#" */
    0x7, 0x55, 0x70,

    /* U+24 "$" */
    0x2, 0x22, 0x33, 0x60,

    /* U+25 "%" */
    0x44, 0x20, 0x62,

    /* U+26 "&" */
    0x6a, 0x63, 0x87,

    /* U+27 "'" */
    0x10,

    /* U+28 "(" */
    0x4a, 0x10,

    /* U+29 ")" */
    0x15, 0x40,

    /* U+2A "*" */
    0x6, 0x60,

    /* U+2B "+" */
    0x2, 0x20,

    /* U+2C "," */
    0x0,

    /* U+2D "-" */
    0x0,

    /* U+2E "." */
    0x0,

    /* U+2F "/" */
    0x0, 0x20, 0x40,

    /* U+30 "0" */
    0x79, 0xbd, 0x97,

    /* U+31 "1" */
    0x9, 0x24, 0x80,

    /* U+32 "2" */
    0x79, 0x12, 0x47,

    /* U+33 "3" */
    0x71, 0x11, 0x17,

    /* U+34 "4" */
    0x13, 0x59, 0x70,

    /* U+35 "5" */
    0x78, 0xc1, 0x16,

    /* U+36 "6" */
    0x68, 0xc9, 0x97,

    /* U+37 "7" */
    0x79, 0x2, 0x20,

    /* U+38 "8" */
    0x71, 0x51, 0x17,

    /* U+39 "9" */
    0x79, 0x97, 0x17,

    /* U+3A ":" */
    0x80,

    /* U+3B ";" */
    0x40, 0x0,

    /* U+3C "<" */
    0x2, 0x44, 0x20,

    /* U+3D "=" */
    0x70, 0x0,

    /* U+3E ">" */
    0x42, 0x11, 0x24,

    /* U+3F "?" */
    0x79, 0x12, 0x0,

    /* U+40 "@" */
    0x7b, 0xdd, 0xb7,

    /* U+41 "A" */
    0x79, 0x9f, 0x90,

    /* U+42 "B" */
    0x79, 0xe9, 0x97,

    /* U+43 "C" */
    0x78, 0x88, 0x87,

    /* U+44 "D" */
    0x69, 0x99, 0x96,

    /* U+45 "E" */
    0x78, 0x88, 0x87,

    /* U+46 "F" */
    0x78, 0x88, 0x80,

    /* U+47 "G" */
    0x78, 0x89, 0x97,

    /* U+48 "H" */
    0x9, 0x99, 0x90,

    /* U+49 "I" */
    0x55, 0x50,

    /* U+4A "J" */
    0x10, 0x84, 0x21, 0x30,

    /* U+4B "K" */
    0x1a, 0xcc, 0xa1,

    /* U+4C "L" */
    0x8, 0x88, 0x87,

    /* U+4D "M" */
    0x99, 0x99, 0x90,

    /* U+4E "N" */
    0x9, 0xdb, 0x90,

    /* U+4F "O" */
    0x79, 0x99, 0x97,

    /* U+50 "P" */
    0x79, 0x9e, 0x80,

    /* U+51 "Q" */
    0x74, 0xa5, 0x28, 0x38, 0x0,

    /* U+52 "R" */
    0x79, 0x9e, 0xa1,

    /* U+53 "S" */
    0x78, 0x41, 0x17,

    /* U+54 "T" */
    0x72, 0x22, 0x20,

    /* U+55 "U" */
    0x9, 0x99, 0x97,

    /* U+56 "V" */
    0x1, 0x54, 0x42,

    /* U+57 "W" */
    0x9, 0x99, 0xd9,

    /* U+58 "X" */
    0x4, 0x66, 0x40,

    /* U+59 "Y" */
    0x15, 0x22, 0x20,

    /* U+5A "Z" */
    0x71, 0x24, 0x7,

    /* U+5B "[" */
    0xea, 0xb0,

    /* U+5C "\\" */
    0x4, 0x2, 0x0,

    /* U+5D "]" */
    0x55, 0x50,

    /* U+5E "^" */
    0x6, 0x0,

    /* U+5F "_" */
    0x70,

    /* U+60 "`" */
    0x10,

    /* U+61 "a" */
    0x77, 0x97,

    /* U+62 "b" */
    0x8, 0xf9, 0x97,

    /* U+63 "c" */
    0x78, 0x87,

    /* U+64 "d" */
    0x1, 0x79, 0x97,

    /* U+65 "e" */
    0x7d, 0x87,

    /* U+66 "f" */
    0x31, 0x18, 0x42, 0x0,

    /* U+67 "g" */
    0x79, 0x97, 0x10,

    /* U+68 "h" */
    0x8, 0xf9, 0x90,

    /* U+69 "i" */
    0x5, 0x50,

    /* U+6A "j" */
    0x10, 0x11, 0x11, 0x50,

    /* U+6B "k" */
    0x4, 0x44, 0x60,

    /* U+6C "l" */
    0x15, 0x50,

    /* U+6D "m" */
    0xfb, 0xb0,

    /* U+6E "n" */
    0xf9, 0x90,

    /* U+6F "o" */
    0x79, 0x97,

    /* U+70 "p" */
    0xf9, 0x9f, 0x80,

    /* U+71 "q" */
    0x79, 0x97, 0x10,

    /* U+72 "r" */
    0xf8, 0x80,

    /* U+73 "s" */
    0x74, 0x17,

    /* U+74 "t" */
    0x0, 0x60, 0x3,

    /* U+75 "u" */
    0x99, 0x97,

    /* U+76 "v" */
    0x15, 0x42,

    /* U+77 "w" */
    0x9b, 0xb7,

    /* U+78 "x" */
    0x16, 0x61,

    /* U+79 "y" */
    0x99, 0x97, 0x10,

    /* U+7A "z" */
    0x72, 0x47,

    /* U+7B "{" */
    0x20, 0x0, 0x2,

    /* U+7C "|" */
    0x15, 0x40,

    /* U+7D "}" */
    0x42, 0x22, 0x24,

    /* U+7E "~" */
    0x3, 0x0,

    /* U+A0 " " */

    /* U+A1 "¡" */
    0x38,

    /* U+A2 "¢" */
    0x27, 0x22, 0x70,

    /* U+A3 "£" */
    0x64, 0x44, 0x47,

    /* U+A4 "¤" */
    0x55, 0x55,

    /* U+A5 "¥" */
    0x4, 0x22, 0x22,

    /* U+A6 "¦" */
    0x48,

    /* U+A7 "§" */
    0x4, 0x44, 0x42, 0x60,

    /* U+A8 "¨" */
    0x14,

    /* U+A9 "©" */
    0x7, 0xb5, 0xaf, 0x0,

    /* U+AA "ª" */
    0x67, 0x50, 0x0,

    /* U+AB "«" */
    0x73, 0x18, 0x20,

    /* U+AC "¬" */
    0x71, 0x0,

    /* U+AD "­" */
    0x0,

    /* U+AE "®" */
    0x7, 0xb5, 0xef, 0x0,

    /* U+AF "¯" */
    0x40,

    /* U+B0 "°" */
    0x13, 0x0,

    /* U+B1 "±" */
    0x2, 0x20, 0x70,

    /* U+B2 "²" */
    0x81, 0x20,

    /* U+B3 "³" */
    0x88, 0x0,

    /* U+B4 "´" */
    0x8,

    /* U+B5 "µ" */
    0x99, 0x9f, 0x80,

    /* U+B6 "¶" */
    0x7b, 0xb3, 0x30,

    /* U+B7 "·" */
    0x0,

    /* U+B8 "¸" */
    0x0,

    /* U+B9 "¹" */
    0x15,

    /* U+BA "º" */
    0x61, 0x50, 0x0,

    /* U+BB "»" */
    0x61, 0x9c, 0x40,

    /* U+BC "¼" */
    0x42, 0x0, 0x22, 0x0,

    /* U+BD "½" */
    0x2, 0x88, 0xa1, 0x8,

    /* U+BE "¾" */
    0xc1, 0x4, 0x66, 0x0,

    /* U+BF "¿" */
    0x0, 0x0, 0x97,

    /* U+C0 "À" */
    0x2, 0x79, 0x9f, 0x90,

    /* U+C1 "Á" */
    0x2, 0x79, 0x9f, 0x90,

    /* U+C2 "Â" */
    0x2, 0x79, 0x9f, 0x90,

    /* U+C3 "Ã" */
    0x6, 0x79, 0x9f, 0x90,

    /* U+C4 "Ä" */
    0x5, 0x79, 0x9f, 0x90,

    /* U+C5 "Å" */
    0x6, 0x79, 0x9f, 0x90,

    /* U+C6 "Æ" */
    0x75, 0x29, 0x4a, 0x18,

    /* U+C7 "Ç" */
    0x78, 0x80, 0x7, 0x0,

    /* U+C8 "È" */
    0x2, 0x78, 0x88, 0x87,

    /* U+C9 "É" */
    0x2, 0x78, 0x88, 0x87,

    /* U+CA "Ê" */
    0x2, 0x78, 0x88, 0x87,

    /* U+CB "Ë" */
    0x5, 0x78, 0x88, 0x87,

    /* U+CC "Ì" */
    0x15, 0x55,

    /* U+CD "Í" */
    0x9, 0x24, 0x92,

    /* U+CE "Î" */
    0x9, 0x24, 0x92,

    /* U+CF "Ï" */
    0x15, 0x24, 0x92,

    /* U+D0 "Ð" */
    0x69, 0x99, 0x96,

    /* U+D1 "Ñ" */
    0x6, 0x9, 0xdb, 0x90,

    /* U+D2 "Ò" */
    0x2, 0x79, 0x99, 0x97,

    /* U+D3 "Ó" */
    0x2, 0x79, 0x99, 0x97,

    /* U+D4 "Ô" */
    0x2, 0x79, 0x99, 0x97,

    /* U+D5 "Õ" */
    0x6, 0x79, 0x99, 0x97,

    /* U+D6 "Ö" */
    0x5, 0x79, 0x99, 0x97,

    /* U+D7 "×" */
    0x6, 0x61,

    /* U+D8 "Ø" */
    0x79, 0xbd, 0x96,

    /* U+D9 "Ù" */
    0x2, 0x9, 0x99, 0x97,

    /* U+DA "Ú" */
    0x2, 0x9, 0x99, 0x97,

    /* U+DB "Û" */
    0x2, 0x9, 0x99, 0x97,

    /* U+DC "Ü" */
    0x5, 0x9, 0x99, 0x97,

    /* U+DD "Ý" */
    0x2, 0x15, 0x22, 0x20,

    /* U+DE "Þ" */
    0xe, 0x99, 0xe0,

    /* U+DF "ß" */
    0x69, 0xe9, 0x97,

    /* U+E0 "à" */
    0x0, 0x77, 0x97,

    /* U+E1 "á" */
    0x20, 0x77, 0x97,

    /* U+E2 "â" */
    0x20, 0x77, 0x97,

    /* U+E3 "ã" */
    0x40, 0x77, 0x97,

    /* U+E4 "ä" */
    0x0, 0x77, 0x97,

    /* U+E5 "å" */
    0x20, 0x77, 0x97,

    /* U+E6 "æ" */
    0x73, 0xa8, 0xe0,

    /* U+E7 "ç" */
    0x78, 0x7, 0x0,

    /* U+E8 "è" */
    0x0, 0x7d, 0x87,

    /* U+E9 "é" */
    0x20, 0x7d, 0x87,

    /* U+EA "ê" */
    0x20, 0x7d, 0x87,

    /* U+EB "ë" */
    0x0, 0x7d, 0x87,

    /* U+EC "ì" */
    0x85, 0x50,

    /* U+ED "í" */
    0x41, 0x24, 0x80,

    /* U+EE "î" */
    0x41, 0x24, 0x80,

    /* U+EF "ï" */
    0x1, 0x24, 0x80,

    /* U+F0 "ð" */
    0x42, 0x79, 0x97,

    /* U+F1 "ñ" */
    0x40, 0xf9, 0x90,

    /* U+F2 "ò" */
    0x0, 0x79, 0x97,

    /* U+F3 "ó" */
    0x20, 0x79, 0x97,

    /* U+F4 "ô" */
    0x20, 0x79, 0x97,

    /* U+F5 "õ" */
    0x40, 0x79, 0x97,

    /* U+F6 "ö" */
    0x0, 0x79, 0x97,

    /* U+F7 "÷" */
    0x20, 0x2,

    /* U+F8 "ø" */
    0x7b, 0xd6,

    /* U+F9 "ù" */
    0x0, 0x99, 0x97,

    /* U+FA "ú" */
    0x20, 0x99, 0x97,

    /* U+FB "û" */
    0x20, 0x99, 0x97,

    /* U+FC "ü" */
    0x0, 0x99, 0x97,

    /* U+FD "ý" */
    0x20, 0x99, 0x97, 0x10,

    /* U+FE "þ" */
    0x8, 0xf9, 0x9f, 0x80,

    /* U+FF "ÿ" */
    0x0, 0x99, 0x97, 0x10,

    /* U+100 "Ā" */
    0x67, 0x99, 0xf9, 0x0,

    /* U+101 "ā" */
    0x0, 0x77, 0x97,

    /* U+102 "Ă" */
    0x6, 0x79, 0x9f, 0x90,

    /* U+103 "ă" */
    0x0, 0x77, 0x97,

    /* U+104 "Ą" */
    0x79, 0x9f, 0x91, 0x0,

    /* U+105 "ą" */
    0x77, 0x97, 0x0,

    /* U+106 "Ć" */
    0x2, 0x78, 0x88, 0x87,

    /* U+107 "ć" */
    0x20, 0x78, 0x87,

    /* U+108 "Ĉ" */
    0x2, 0x78, 0x88, 0x87,

    /* U+109 "ĉ" */
    0x20, 0x78, 0x87,

    /* U+10A "Ċ" */
    0x2, 0x78, 0x88, 0x87,

    /* U+10B "ċ" */
    0x0, 0x78, 0x87,

    /* U+10C "Č" */
    0x27, 0x88, 0x88, 0x70,

    /* U+10D "č" */
    0x20, 0x78, 0x87,

    /* U+10E "Ď" */
    0x26, 0x99, 0x99, 0x60,

    /* U+10F "ď" */
    0x20, 0x17, 0x99, 0x70,

    /* U+110 "Đ" */
    0x69, 0x99, 0x96,

    /* U+111 "đ" */
    0x0, 0x9d, 0x29, 0x38,

    /* U+112 "Ē" */
    0x67, 0x88, 0x88, 0x70,

    /* U+113 "ē" */
    0x0, 0x7d, 0x87,

    /* U+114 "Ĕ" */
    0x6, 0x78, 0x88, 0x87,

    /* U+115 "ĕ" */
    0x0, 0x7d, 0x87,

    /* U+116 "Ė" */
    0x2, 0x78, 0x88, 0x87,

    /* U+117 "ė" */
    0x0, 0x7d, 0x87,

    /* U+118 "Ę" */
    0x78, 0x88, 0x87, 0x0,

    /* U+119 "ę" */
    0x7d, 0x7, 0x20,

    /* U+11A "Ě" */
    0x27, 0x88, 0x88, 0x70,

    /* U+11B "ě" */
    0x20, 0x7d, 0x87,

    /* U+11C "Ĝ" */
    0x2, 0x78, 0x89, 0x97,

    /* U+11D "ĝ" */
    0x20, 0x79, 0x97, 0x10,

    /* U+11E "Ğ" */
    0x6, 0x78, 0x89, 0x97,

    /* U+11F "ğ" */
    0x0, 0x79, 0x97, 0x10,

    /* U+120 "Ġ" */
    0x2, 0x78, 0x89, 0x97,

    /* U+121 "ġ" */
    0x0, 0x79, 0x97, 0x10,

    /* U+122 "Ģ" */
    0x78, 0x89, 0x97, 0x20,

    /* U+123 "ģ" */
    0x0, 0x79, 0x97, 0x10,

    /* U+124 "Ĥ" */
    0x2, 0x9, 0x99, 0x90,

    /* U+125 "ĥ" */
    0x2, 0x8, 0xf9, 0x90,

    /* U+126 "Ħ" */
    0xf, 0x99, 0x90,

    /* U+127 "ħ" */
    0x88, 0xf9, 0x90,

    /* U+128 "Ĩ" */
    0x6, 0x22, 0x22, 0x22,

    /* U+129 "ĩ" */
    0x40, 0x22, 0x22,

    /* U+12A "Ī" */
    0xc9, 0x24, 0x90,

    /* U+12B "ī" */
    0x1, 0x24, 0x80,

    /* U+12C "Ĭ" */
    0x19, 0x24, 0x92,

    /* U+12D "ĭ" */
    0x1, 0x24, 0x80,

    /* U+12E "Į" */
    0x55, 0x54,

    /* U+12F "į" */
    0x5, 0x54,

    /* U+130 "İ" */
    0x15, 0x55,

    /* U+131 "ı" */
    0x55,

    /* U+132 "Ĳ" */
    0x99, 0x99, 0x9b,

    /* U+133 "ĳ" */
    0x0, 0x25, 0x8, 0x0, 0xc0,

    /* U+134 "Ĵ" */
    0x0, 0x84, 0x21, 0x8, 0x4c,

    /* U+135 "ĵ" */
    0x10, 0x4, 0x21, 0x9, 0x40,

    /* U+136 "Ķ" */
    0x1a, 0xcc, 0xa1, 0x20,

    /* U+137 "ķ" */
    0x4, 0x44, 0x60, 0x20,

    /* U+138 "ĸ" */
    0x44, 0x60,

    /* U+139 "Ĺ" */
    0x4, 0x8, 0x88, 0x87,

    /* U+13A "ĺ" */
    0x8, 0x24, 0x92,

    /* U+13B "Ļ" */
    0x8, 0x88, 0x87, 0x20,

    /* U+13C "ļ" */
    0x15, 0x54,

    /* U+13D "Ľ" */
    0x20, 0x88, 0x88, 0x70,

    /* U+13E "ľ" */
    0x41, 0x24, 0x90,

    /* U+13F "Ŀ" */
    0x8, 0x88, 0x87,

    /* U+140 "ŀ" */
    0x4, 0x44, 0x44,

    /* U+141 "Ł" */
    0x8, 0xc8, 0x87,

    /* U+142 "ł" */
    0x9, 0x24, 0x80,

    /* U+143 "Ń" */
    0x2, 0x9, 0xdb, 0x90,

    /* U+144 "ń" */
    0x20, 0xf9, 0x90,

    /* U+145 "Ņ" */
    0x9, 0xdb, 0x90, 0x20,

    /* U+146 "ņ" */
    0xf9, 0x90, 0x20,

    /* U+147 "Ň" */
    0x20, 0x9d, 0xb9, 0x0,

    /* U+148 "ň" */
    0x20, 0xf9, 0x90,

    /* U+149 "ŉ" */
    0x8, 0xf, 0x99, 0x0,

    /* U+14A "Ŋ" */
    0x9, 0xdb, 0x91, 0x10,

    /* U+14B "ŋ" */
    0xf9, 0x91, 0x10,

    /* U+14C "Ō" */
    0x67, 0x99, 0x99, 0x70,

    /* U+14D "ō" */
    0x0, 0x79, 0x97,

    /* U+14E "Ŏ" */
    0x6, 0x79, 0x99, 0x97,

    /* U+14F "ŏ" */
    0x0, 0x79, 0x97,

    /* U+150 "Ő" */
    0x2, 0x9d, 0x29, 0x4a, 0x4e,

    /* U+151 "ő" */
    0x10, 0x1d, 0x29, 0x38,

    /* U+152 "Œ" */
    0x75, 0x29, 0x4a, 0x38,

    /* U+153 "œ" */
    0xf5, 0xa8, 0xe0,

    /* U+154 "Ŕ" */
    0x2, 0x79, 0x9e, 0xa1,

    /* U+155 "ŕ" */
    0x20, 0xf8, 0x80,

    /* U+156 "Ŗ" */
    0x79, 0x9e, 0xa1, 0x20,

    /* U+157 "ŗ" */
    0xf8, 0x80, 0x80,

    /* U+158 "Ř" */
    0x27, 0x99, 0xea, 0x10,

    /* U+159 "ř" */
    0x20, 0xf8, 0x80,

    /* U+15A "Ś" */
    0x2, 0x78, 0x41, 0x17,

    /* U+15B "ś" */
    0x20, 0x74, 0x17,

    /* U+15C "Ŝ" */
    0x2, 0x78, 0x41, 0x17,

    /* U+15D "ŝ" */
    0x20, 0x74, 0x17,

    /* U+15E "Ş" */
    0x78, 0x41, 0x16, 0x0,

    /* U+15F "ş" */
    0x74, 0x17, 0x0,

    /* U+160 "Š" */
    0x27, 0x84, 0x11, 0x70,

    /* U+161 "š" */
    0x20, 0x74, 0x17,

    /* U+162 "Ţ" */
    0x72, 0x22, 0x22, 0x20,

    /* U+163 "ţ" */
    0x0, 0x60, 0x3, 0x0,

    /* U+164 "Ť" */
    0x27, 0x22, 0x22, 0x0,

    /* U+165 "ť" */
    0x60, 0x6, 0x0, 0x30,

    /* U+166 "Ŧ" */
    0x72, 0x22, 0x20,

    /* U+167 "ŧ" */
    0x0, 0x66, 0x3,

    /* U+168 "Ũ" */
    0x6, 0x9, 0x99, 0x97,

    /* U+169 "ũ" */
    0x40, 0x99, 0x97,

    /* U+16A "Ū" */
    0x60, 0x99, 0x99, 0x70,

    /* U+16B "ū" */
    0x0, 0x99, 0x97,

    /* U+16C "Ŭ" */
    0x6, 0x9, 0x99, 0x97,

    /* U+16D "ŭ" */
    0x0, 0x99, 0x97,

    /* U+16E "Ů" */
    0x6, 0x9, 0x99, 0x97,

    /* U+16F "ů" */
    0x20, 0x99, 0x97,

    /* U+170 "Ű" */
    0x2, 0x81, 0x29, 0x4a, 0x4e,

    /* U+171 "ű" */
    0x10, 0x25, 0x29, 0x38,

    /* U+172 "Ų" */
    0x9, 0x91, 0x16, 0x20,

    /* U+173 "ų" */
    0x99, 0x97, 0x0,

    /* U+174 "Ŵ" */
    0x2, 0x9, 0x99, 0xd9,

    /* U+175 "ŵ" */
    0x20, 0x9b, 0xb7,

    /* U+176 "Ŷ" */
    0x2, 0x15, 0x22, 0x20,

    /* U+177 "ŷ" */
    0x20, 0x99, 0x97, 0x10,

    /* U+178 "Ÿ" */
    0x5, 0x15, 0x22, 0x20,

    /* U+179 "Ź" */
    0x2, 0x71, 0x24, 0x7,

    /* U+17A "ź" */
    0x20, 0x72, 0x47,

    /* U+17B "Ż" */
    0x2, 0x71, 0x24, 0x7,

    /* U+17C "ż" */
    0x0, 0x72, 0x47,

    /* U+17D "Ž" */
    0x27, 0x12, 0x40, 0x70,

    /* U+17E "ž" */
    0x20, 0x72, 0x47,

    /* U+17F "ſ" */
    0x60, 0x0, 0x0
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 72, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 72, .box_h = 6, .box_w = 1, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 72, .box_h = 3, .box_w = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 3, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 6, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 10, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 13, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 72, .box_h = 3, .box_w = 2, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 17, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 19, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 23, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 25, .adv_w = 72, .box_h = 3, .box_w = 2, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 26, .adv_w = 72, .box_h = 2, .box_w = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 27, .adv_w = 72, .box_h = 2, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 28, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 31, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 34, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 37, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 40, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 43, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 46, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 49, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 52, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 58, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 61, .adv_w = 72, .box_h = 4, .box_w = 1, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 72, .box_h = 5, .box_w = 2, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 64, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 67, .adv_w = 72, .box_h = 3, .box_w = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 69, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 72, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 75, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 78, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 81, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 84, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 87, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 90, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 93, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 96, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 99, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 102, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 104, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 108, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 111, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 114, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 117, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 120, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 123, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 126, .adv_w = 72, .box_h = 7, .box_w = 5, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 131, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 134, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 137, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 140, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 143, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 149, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 152, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 158, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 160, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 163, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 165, .adv_w = 72, .box_h = 3, .box_w = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 167, .adv_w = 72, .box_h = 1, .box_w = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 168, .adv_w = 72, .box_h = 2, .box_w = 2, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 169, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 171, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 174, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 176, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 185, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 188, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 191, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 193, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 197, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 200, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 206, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 208, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 211, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 214, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 216, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 218, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 221, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 223, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 232, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 234, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 237, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 239, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 242, .adv_w = 72, .box_h = 3, .box_w = 4, .ofs_x = 0, .ofs_y = 4},
    {.bitmap_index = 244, .adv_w = 72, .box_h = 0, .box_w = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 244, .adv_w = 72, .box_h = 6, .box_w = 1, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 245, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 248, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 251, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 253, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 256, .adv_w = 72, .box_h = 6, .box_w = 1, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 257, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 261, .adv_w = 72, .box_h = 2, .box_w = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 262, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 72, .box_h = 5, .box_w = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 269, .adv_w = 72, .box_h = 4, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 272, .adv_w = 72, .box_h = 3, .box_w = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 274, .adv_w = 72, .box_h = 2, .box_w = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 275, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 279, .adv_w = 72, .box_h = 1, .box_w = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 280, .adv_w = 72, .box_h = 3, .box_w = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 282, .adv_w = 72, .box_h = 5, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 285, .adv_w = 72, .box_h = 4, .box_w = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 287, .adv_w = 72, .box_h = 4, .box_w = 3, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 289, .adv_w = 72, .box_h = 2, .box_w = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 290, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 293, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 296, .adv_w = 72, .box_h = 2, .box_w = 2, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 297, .adv_w = 72, .box_h = 3, .box_w = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 298, .adv_w = 72, .box_h = 4, .box_w = 2, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 299, .adv_w = 72, .box_h = 5, .box_w = 4, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 302, .adv_w = 72, .box_h = 4, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 305, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 309, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 313, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 317, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 320, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 324, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 328, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 332, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 336, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 340, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 348, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 352, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 356, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 360, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 368, .adv_w = 72, .box_h = 8, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 370, .adv_w = 72, .box_h = 8, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 72, .box_h = 8, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 376, .adv_w = 72, .box_h = 8, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 379, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 382, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 386, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 390, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 394, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 398, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 402, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 406, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 408, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 411, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 419, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 423, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 427, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 431, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 437, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 440, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 443, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 446, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 449, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 452, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 455, .adv_w = 72, .box_h = 4, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 458, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 461, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 464, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 467, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 470, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 72, .box_h = 6, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 475, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 478, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 481, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 487, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 490, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 493, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 499, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 502, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 505, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 507, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 509, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 512, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 515, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 518, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 521, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 525, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 529, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 533, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 537, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 540, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 544, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 547, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 551, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 554, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 558, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 561, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 565, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 572, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 575, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 579, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 582, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 586, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 590, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 593, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 597, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 601, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 604, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 608, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 611, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 615, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 618, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 622, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 625, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 629, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 632, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 636, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 640, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 644, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 648, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 652, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 656, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 660, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 664, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 668, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 672, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 675, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 678, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 682, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 685, .adv_w = 72, .box_h = 7, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 688, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 691, .adv_w = 72, .box_h = 8, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 694, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 697, .adv_w = 72, .box_h = 8, .box_w = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 699, .adv_w = 72, .box_h = 8, .box_w = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 701, .adv_w = 72, .box_h = 8, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 703, .adv_w = 72, .box_h = 4, .box_w = 2, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 704, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 707, .adv_w = 72, .box_h = 8, .box_w = 5, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 712, .adv_w = 72, .box_h = 8, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 717, .adv_w = 72, .box_h = 8, .box_w = 5, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 722, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 726, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 730, .adv_w = 72, .box_h = 4, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 732, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 736, .adv_w = 72, .box_h = 8, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 739, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 743, .adv_w = 72, .box_h = 8, .box_w = 2, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 745, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 749, .adv_w = 72, .box_h = 7, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 752, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 755, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 758, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 761, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 764, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 768, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 771, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 775, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 778, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 782, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 785, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 789, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 793, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 796, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 800, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 803, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 807, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 810, .adv_w = 72, .box_h = 8, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 815, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 819, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 823, .adv_w = 72, .box_h = 4, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 826, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 830, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 833, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 837, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 840, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 844, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 847, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 851, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 854, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 858, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 861, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 865, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 868, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 872, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 875, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 879, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 883, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 887, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 891, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 894, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 897, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 901, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 904, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 908, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 911, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 915, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 918, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 922, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 925, .adv_w = 72, .box_h = 8, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 930, .adv_w = 72, .box_h = 6, .box_w = 5, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 934, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 938, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 941, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 945, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 948, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 952, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = -2},
    {.bitmap_index = 956, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 960, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 964, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 967, .adv_w = 72, .box_h = 8, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 971, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 974, .adv_w = 72, .box_h = 7, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 978, .adv_w = 72, .box_h = 6, .box_w = 4, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 981, .adv_w = 72, .box_h = 6, .box_w = 3, .ofs_x = 1, .ofs_y = 0}
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
    },
    {
        .range_start = 160, .range_length = 224, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
        .glyph_id_start = 96, .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0
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
lv_font_t terminus_9 = {
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .line_height = 10,          /*The maximum line height required by the font*/
    .base_line = 2,             /*Baseline measured from the bottom of the line*/
};

#endif /*#if TERMINUS_9*/

