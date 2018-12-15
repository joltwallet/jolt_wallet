
#include "lvgl/lv_misc/lv_font.h"

#if USE_LV_FONT_PIXELMIX_7 != 0  /*Can be enabled in lv_conf.h*/

/***********************************************************************************
 * pixelmix.ttf 7 px Font in U+0020 ( ) .. U+007f ()  range with 1 bpp
***********************************************************************************/

/*Store the image of the letters (glyph)*/
static const uint8_t pixelmix_7_glyph_bitmap[] = 
{
  /*Unicode: U+0020 ( ) , Width: 2 */
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 


  /*Unicode: U+0021 (!) , Width: 1 */
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 
  0x00,  //. 
  0x80,  //% 


  /*Unicode: U+0022 (") , Width: 3 */
  0xa0,  //%.% 
  0xa0,  //%.% 
  0x00,  //... 
  0x00,  //... 
  0x00,  //... 
  0x00,  //... 
  0x00,  //... 


  /*Unicode: U+0023 (#) , Width: 4 */
  0x50,  //.%.% 
  0xf0,  //%%%% 
  0x50,  //.%.% 
  0x50,  //.%.% 
  0x50,  //.%.% 
  0xf0,  //%%%% 
  0x50,  //.%.% 


  /*Unicode: U+0024 ($) , Width: 5 */
  0x70,  //.%%%. 
  0xa8,  //%.%.% 
  0xa0,  //%.%.. 
  0x70,  //.%%%. 
  0x28,  //..%.% 
  0xa8,  //%.%.% 
  0x70,  //.%%%. 


  /*Unicode: U+0025 (%) , Width: 5 */
  0xc8,  //%%..% 
  0xd0,  //%%.%. 
  0x10,  //...%. 
  0x20,  //..%.. 
  0x40,  //.%... 
  0x58,  //.%.%% 
  0x98,  //%..%% 


  /*Unicode: U+0026 (&) , Width: 5 */
  0xe0,  //%%%.. 
  0xa0,  //%.%.. 
  0xa0,  //%.%.. 
  0x40,  //.%... 
  0xb0,  //%.%%. 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0027 (') , Width: 1 */
  0x80,  //% 
  0x80,  //% 
  0x00,  //. 
  0x00,  //. 
  0x00,  //. 
  0x00,  //. 
  0x00,  //. 


  /*Unicode: U+0028 (() , Width: 3 */
  0x20,  //..% 
  0x40,  //.%. 
  0x80,  //%.. 
  0x80,  //%.. 
  0x80,  //%.. 
  0x40,  //.%. 
  0x20,  //..% 


  /*Unicode: U+0029 ()) , Width: 3 */
  0x80,  //%.. 
  0x40,  //.%. 
  0x20,  //..% 
  0x20,  //..% 
  0x20,  //..% 
  0x40,  //.%. 
  0x80,  //%.. 


  /*Unicode: U+002a (*) , Width: 3 */
  0x40,  //.%. 
  0xe0,  //%%% 
  0x40,  //.%. 
  0x00,  //... 
  0x00,  //... 
  0x00,  //... 
  0x00,  //... 


  /*Unicode: U+002b (+) , Width: 4 */
  0x00,  //.... 
  0x20,  //..%. 
  0x20,  //..%. 
  0xf0,  //%%%% 
  0x20,  //..%. 
  0x20,  //..%. 
  0x00,  //.... 


  /*Unicode: U+002c (,) , Width: 2 */
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0xc0,  //%% 
  0x40,  //.% 


  /*Unicode: U+002d (-) , Width: 4 */
  0x00,  //.... 
  0x00,  //.... 
  0x00,  //.... 
  0xf0,  //%%%% 
  0x00,  //.... 
  0x00,  //.... 
  0x00,  //.... 


  /*Unicode: U+002e (.) , Width: 2 */
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0xc0,  //%% 
  0xc0,  //%% 


  /*Unicode: U+002f (/) , Width: 3 */
  0x20,  //..% 
  0x20,  //..% 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x80,  //%.. 
  0x80,  //%.. 


  /*Unicode: U+0030 (0) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x98,  //%..%% 
  0xa8,  //%.%.% 
  0xc8,  //%%..% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0031 (1) , Width: 3 */
  0x40,  //.%. 
  0xc0,  //%%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0xe0,  //%%% 


  /*Unicode: U+0032 (2) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x08,  //....% 
  0x10,  //...%. 
  0x20,  //..%.. 
  0x40,  //.%... 
  0xf8,  //%%%%% 


  /*Unicode: U+0033 (3) , Width: 5 */
  0xf8,  //%%%%% 
  0x10,  //...%. 
  0x20,  //..%.. 
  0x10,  //...%. 
  0x08,  //....% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0034 (4) , Width: 5 */
  0x10,  //...%. 
  0x30,  //..%%. 
  0x50,  //.%.%. 
  0x90,  //%..%. 
  0xf8,  //%%%%% 
  0x10,  //...%. 
  0x10,  //...%. 


  /*Unicode: U+0035 (5) , Width: 5 */
  0xf8,  //%%%%% 
  0x80,  //%.... 
  0xf0,  //%%%%. 
  0x08,  //....% 
  0x08,  //....% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0036 (6) , Width: 5 */
  0x30,  //..%%. 
  0x40,  //.%... 
  0x80,  //%.... 
  0xf0,  //%%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0037 (7) , Width: 5 */
  0xf8,  //%%%%% 
  0x08,  //....% 
  0x10,  //...%. 
  0x20,  //..%.. 
  0x40,  //.%... 
  0x40,  //.%... 
  0x40,  //.%... 


  /*Unicode: U+0038 (8) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0039 (9) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x78,  //.%%%% 
  0x08,  //....% 
  0x10,  //...%. 
  0x60,  //.%%.. 


  /*Unicode: U+003a (:) , Width: 1 */
  0x00,  //. 
  0x80,  //% 
  0x00,  //. 
  0x00,  //. 
  0x00,  //. 
  0x80,  //% 
  0x00,  //. 


  /*Unicode: U+003b (;) , Width: 2 */
  0x00,  //.. 
  0x40,  //.% 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0xc0,  //%% 
  0x40,  //.% 


  /*Unicode: U+003c (<) , Width: 4 */
  0x10,  //...% 
  0x20,  //..%. 
  0x40,  //.%.. 
  0x80,  //%... 
  0x40,  //.%.. 
  0x20,  //..%. 
  0x10,  //...% 


  /*Unicode: U+003d (=) , Width: 4 */
  0x00,  //.... 
  0x00,  //.... 
  0xf0,  //%%%% 
  0x00,  //.... 
  0xf0,  //%%%% 
  0x00,  //.... 
  0x00,  //.... 


  /*Unicode: U+003e (>) , Width: 4 */
  0x80,  //%... 
  0x40,  //.%.. 
  0x20,  //..%. 
  0x10,  //...% 
  0x20,  //..%. 
  0x40,  //.%.. 
  0x80,  //%... 


  /*Unicode: U+003f (?) , Width: 4 */
  0x60,  //.%%. 
  0x90,  //%..% 
  0x10,  //...% 
  0x20,  //..%. 
  0x40,  //.%.. 
  0x00,  //.... 
  0x40,  //.%.. 


  /*Unicode: U+0040 (@) , Width: 6 */
  0x78,  //.%%%%. 
  0x84,  //%....% 
  0xbc,  //%.%%%% 
  0xac,  //%.%.%% 
  0xbc,  //%.%%%% 
  0x80,  //%..... 
  0x78,  //.%%%%. 


  /*Unicode: U+0041 (A) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf8,  //%%%%% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+0042 (B) , Width: 5 */
  0xf0,  //%%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf0,  //%%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf0,  //%%%%. 


  /*Unicode: U+0043 (C) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0044 (D) , Width: 5 */
  0xf0,  //%%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf0,  //%%%%. 


  /*Unicode: U+0045 (E) , Width: 5 */
  0xf0,  //%%%%. 
  0x80,  //%.... 
  0x80,  //%.... 
  0xf0,  //%%%%. 
  0x80,  //%.... 
  0x80,  //%.... 
  0xf0,  //%%%%. 


  /*Unicode: U+0046 (F) , Width: 5 */
  0xf0,  //%%%%. 
  0x80,  //%.... 
  0x80,  //%.... 
  0xf0,  //%%%%. 
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 


  /*Unicode: U+0047 (G) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x80,  //%.... 
  0xb8,  //%.%%% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x78,  //.%%%% 


  /*Unicode: U+0048 (H) , Width: 5 */
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf8,  //%%%%% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+0049 (I) , Width: 3 */
  0xe0,  //%%% 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0xe0,  //%%% 


  /*Unicode: U+004a (J) , Width: 5 */
  0x38,  //..%%% 
  0x10,  //...%. 
  0x10,  //...%. 
  0x10,  //...%. 
  0x10,  //...%. 
  0x90,  //%..%. 
  0x60,  //.%%.. 


  /*Unicode: U+004b (K) , Width: 5 */
  0x88,  //%...% 
  0x90,  //%..%. 
  0xa0,  //%.%.. 
  0xc0,  //%%... 
  0xa0,  //%.%.. 
  0x90,  //%..%. 
  0x88,  //%...% 


  /*Unicode: U+004c (L) , Width: 5 */
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 
  0xf0,  //%%%%. 


  /*Unicode: U+004d (M) , Width: 5 */
  0x88,  //%...% 
  0xd8,  //%%.%% 
  0xa8,  //%.%.% 
  0xa8,  //%.%.% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+004e (N) , Width: 5 */
  0x88,  //%...% 
  0x88,  //%...% 
  0xc8,  //%%..% 
  0xa8,  //%.%.% 
  0x98,  //%..%% 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+004f (O) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0050 (P) , Width: 5 */
  0xf0,  //%%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf0,  //%%%%. 
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 


  /*Unicode: U+0051 (Q) , Width: 5 */
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0xa8,  //%.%.% 
  0x90,  //%..%. 
  0x68,  //.%%.% 


  /*Unicode: U+0052 (R) , Width: 5 */
  0xf0,  //%%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf0,  //%%%%. 
  0xa0,  //%.%.. 
  0x90,  //%..%. 
  0x88,  //%...% 


  /*Unicode: U+0053 (S) , Width: 5 */
  0x78,  //.%%%% 
  0x80,  //%.... 
  0x80,  //%.... 
  0x70,  //.%%%. 
  0x08,  //....% 
  0x08,  //....% 
  0xf0,  //%%%%. 


  /*Unicode: U+0054 (T) , Width: 4 */
  0xf0,  //%%%% 
  0x20,  //..%. 
  0x20,  //..%. 
  0x20,  //..%. 
  0x20,  //..%. 
  0x20,  //..%. 
  0x20,  //..%. 


  /*Unicode: U+0055 (U) , Width: 5 */
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0056 (V) , Width: 5 */
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x50,  //.%.%. 
  0x50,  //.%.%. 
  0x20,  //..%.. 


  /*Unicode: U+0057 (W) , Width: 5 */
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0xa8,  //%.%.% 
  0xa8,  //%.%.% 
  0xa8,  //%.%.% 
  0x50,  //.%.%. 


  /*Unicode: U+0058 (X) , Width: 5 */
  0x88,  //%...% 
  0x88,  //%...% 
  0x50,  //.%.%. 
  0x20,  //..%.. 
  0x50,  //.%.%. 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+0059 (Y) , Width: 5 */
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x50,  //.%.%. 
  0x20,  //..%.. 
  0x20,  //..%.. 
  0x20,  //..%.. 


  /*Unicode: U+005a (Z) , Width: 5 */
  0xf8,  //%%%%% 
  0x08,  //....% 
  0x10,  //...%. 
  0x20,  //..%.. 
  0x40,  //.%... 
  0x80,  //%.... 
  0xf8,  //%%%%% 


  /*Unicode: U+005b ([) , Width: 2 */
  0xc0,  //%% 
  0x80,  //%. 
  0x80,  //%. 
  0x80,  //%. 
  0x80,  //%. 
  0x80,  //%. 
  0xc0,  //%% 


  /*Unicode: U+005c (\) , Width: 3 */
  0x80,  //%.. 
  0x80,  //%.. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x20,  //..% 
  0x20,  //..% 


  /*Unicode: U+005d (]) , Width: 2 */
  0xc0,  //%% 
  0x40,  //.% 
  0x40,  //.% 
  0x40,  //.% 
  0x40,  //.% 
  0x40,  //.% 
  0xc0,  //%% 


  /*Unicode: U+005e (^) , Width: 5 */
  0x20,  //..%.. 
  0x50,  //.%.%. 
  0x88,  //%...% 
  0x00,  //..... 
  0x00,  //..... 
  0x00,  //..... 
  0x00,  //..... 


  /*Unicode: U+005f (_) , Width: 4 */
  0x00,  //.... 
  0x00,  //.... 
  0x00,  //.... 
  0x00,  //.... 
  0x00,  //.... 
  0x00,  //.... 
  0xf0,  //%%%% 


  /*Unicode: U+0060 (`) , Width: 2 */
  0x80,  //%. 
  0x40,  //.% 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 
  0x00,  //.. 


  /*Unicode: U+0061 (a) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x70,  //.%%%. 
  0x08,  //....% 
  0x78,  //.%%%% 
  0x88,  //%...% 
  0x78,  //.%%%% 


  /*Unicode: U+0062 (b) , Width: 5 */
  0x80,  //%.... 
  0x80,  //%.... 
  0xb0,  //%.%%. 
  0xc8,  //%%..% 
  0x88,  //%...% 
  0x88,  //%...% 
  0xf0,  //%%%%. 


  /*Unicode: U+0063 (c) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x70,  //.%%%. 
  0x80,  //%.... 
  0x80,  //%.... 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0064 (d) , Width: 5 */
  0x08,  //....% 
  0x08,  //....% 
  0x68,  //.%%.% 
  0x98,  //%..%% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x78,  //.%%%% 


  /*Unicode: U+0065 (e) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x70,  //.%%%. 
  0x88,  //%...% 
  0xf8,  //%%%%% 
  0x80,  //%.... 
  0x70,  //.%%%. 


  /*Unicode: U+0066 (f) , Width: 4 */
  0x40,  //.%.. 
  0x40,  //.%.. 
  0x40,  //.%.. 
  0xe0,  //%%%. 
  0x40,  //.%.. 
  0x40,  //.%.. 
  0x40,  //.%.. 


  /*Unicode: U+0067 (g) , Width: 5 */
  0x00,  //..... 
  0x78,  //.%%%% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x78,  //.%%%% 
  0x08,  //....% 
  0x70,  //.%%%. 


  /*Unicode: U+0068 (h) , Width: 5 */
  0x80,  //%.... 
  0x80,  //%.... 
  0xb0,  //%.%%. 
  0xc8,  //%%..% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+0069 (i) , Width: 3 */
  0x40,  //.%. 
  0x00,  //... 
  0xc0,  //%%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0xe0,  //%%% 


  /*Unicode: U+006a (j) , Width: 4 */
  0x10,  //...% 
  0x00,  //.... 
  0x10,  //...% 
  0x10,  //...% 
  0x10,  //...% 
  0x90,  //%..% 
  0x60,  //.%%. 


  /*Unicode: U+006b (k) , Width: 4 */
  0x80,  //%... 
  0x80,  //%... 
  0x90,  //%..% 
  0xa0,  //%.%. 
  0xc0,  //%%.. 
  0xa0,  //%.%. 
  0x90,  //%..% 


  /*Unicode: U+006c (l) , Width: 3 */
  0xc0,  //%%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0xe0,  //%%% 


  /*Unicode: U+006d (m) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0xd0,  //%%.%. 
  0xa8,  //%.%.% 
  0xa8,  //%.%.% 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+006e (n) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0xb0,  //%.%%. 
  0xc8,  //%%..% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 


  /*Unicode: U+006f (o) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x70,  //.%%%. 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0070 (p) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0xf0,  //%%%%. 
  0x88,  //%...% 
  0xf0,  //%%%%. 
  0x80,  //%.... 
  0x80,  //%.... 


  /*Unicode: U+0071 (q) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x68,  //.%%.% 
  0x98,  //%..%% 
  0x78,  //.%%%% 
  0x08,  //....% 
  0x08,  //....% 


  /*Unicode: U+0072 (r) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0xb0,  //%.%%. 
  0xc8,  //%%..% 
  0x80,  //%.... 
  0x80,  //%.... 
  0x80,  //%.... 


  /*Unicode: U+0073 (s) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x70,  //.%%%. 
  0x80,  //%.... 
  0x70,  //.%%%. 
  0x08,  //....% 
  0xf0,  //%%%%. 


  /*Unicode: U+0074 (t) , Width: 4 */
  0x40,  //.%.. 
  0x40,  //.%.. 
  0xe0,  //%%%. 
  0x40,  //.%.. 
  0x40,  //.%.. 
  0x40,  //.%.. 
  0x30,  //..%% 


  /*Unicode: U+0075 (u) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x70,  //.%%%. 


  /*Unicode: U+0076 (v) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x88,  //%...% 
  0x88,  //%...% 
  0x88,  //%...% 
  0x50,  //.%.%. 
  0x20,  //..%.. 


  /*Unicode: U+0077 (w) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x88,  //%...% 
  0x88,  //%...% 
  0xa8,  //%.%.% 
  0xa8,  //%.%.% 
  0x50,  //.%.%. 


  /*Unicode: U+0078 (x) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x88,  //%...% 
  0x50,  //.%.%. 
  0x20,  //..%.. 
  0x50,  //.%.%. 
  0x88,  //%...% 


  /*Unicode: U+0079 (y) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x88,  //%...% 
  0x88,  //%...% 
  0x78,  //.%%%% 
  0x08,  //....% 
  0x70,  //.%%%. 


  /*Unicode: U+007a (z) , Width: 4 */
  0x00,  //.... 
  0x00,  //.... 
  0xf0,  //%%%% 
  0x10,  //...% 
  0x20,  //..%. 
  0x40,  //.%.. 
  0xf0,  //%%%% 


  /*Unicode: U+007b ({) , Width: 3 */
  0x60,  //.%% 
  0x40,  //.%. 
  0x40,  //.%. 
  0x80,  //%.. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x60,  //.%% 


  /*Unicode: U+007c (|) , Width: 1 */
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 
  0x80,  //% 


  /*Unicode: U+007d (}) , Width: 3 */
  0xc0,  //%%. 
  0x40,  //.%. 
  0x40,  //.%. 
  0x20,  //..% 
  0x40,  //.%. 
  0x40,  //.%. 
  0xc0,  //%%. 


  /*Unicode: U+007e (~) , Width: 5 */
  0x00,  //..... 
  0x00,  //..... 
  0x40,  //.%... 
  0xa8,  //%.%.% 
  0x10,  //...%. 
  0x00,  //..... 
  0x00,  //..... 


  /*Unicode: U+007f () , Width: 7 */
  0xfe,  //%%%%%%% 
  0xfe,  //%%%%%%% 
  0xfe,  //%%%%%%% 
  0xfe,  //%%%%%%% 
  0xfe,  //%%%%%%% 
  0xfe,  //%%%%%%% 
  0xfe,  //%%%%%%% 


};


/*Store the glyph descriptions*/
static const lv_font_glyph_dsc_t pixelmix_7_glyph_dsc[] = 
{
  {.w_px = 2,	.glyph_index = 0},	/*Unicode: U+0020 ( )*/
  {.w_px = 1,	.glyph_index = 7},	/*Unicode: U+0021 (!)*/
  {.w_px = 3,	.glyph_index = 14},	/*Unicode: U+0022 (")*/
  {.w_px = 4,	.glyph_index = 21},	/*Unicode: U+0023 (#)*/
  {.w_px = 5,	.glyph_index = 28},	/*Unicode: U+0024 ($)*/
  {.w_px = 5,	.glyph_index = 35},	/*Unicode: U+0025 (%)*/
  {.w_px = 5,	.glyph_index = 42},	/*Unicode: U+0026 (&)*/
  {.w_px = 1,	.glyph_index = 49},	/*Unicode: U+0027 (')*/
  {.w_px = 3,	.glyph_index = 56},	/*Unicode: U+0028 (()*/
  {.w_px = 3,	.glyph_index = 63},	/*Unicode: U+0029 ())*/
  {.w_px = 3,	.glyph_index = 70},	/*Unicode: U+002a (*)*/
  {.w_px = 4,	.glyph_index = 77},	/*Unicode: U+002b (+)*/
  {.w_px = 2,	.glyph_index = 84},	/*Unicode: U+002c (,)*/
  {.w_px = 4,	.glyph_index = 91},	/*Unicode: U+002d (-)*/
  {.w_px = 2,	.glyph_index = 98},	/*Unicode: U+002e (.)*/
  {.w_px = 3,	.glyph_index = 105},	/*Unicode: U+002f (/)*/
  {.w_px = 5,	.glyph_index = 112},	/*Unicode: U+0030 (0)*/
  {.w_px = 3,	.glyph_index = 119},	/*Unicode: U+0031 (1)*/
  {.w_px = 5,	.glyph_index = 126},	/*Unicode: U+0032 (2)*/
  {.w_px = 5,	.glyph_index = 133},	/*Unicode: U+0033 (3)*/
  {.w_px = 5,	.glyph_index = 140},	/*Unicode: U+0034 (4)*/
  {.w_px = 5,	.glyph_index = 147},	/*Unicode: U+0035 (5)*/
  {.w_px = 5,	.glyph_index = 154},	/*Unicode: U+0036 (6)*/
  {.w_px = 5,	.glyph_index = 161},	/*Unicode: U+0037 (7)*/
  {.w_px = 5,	.glyph_index = 168},	/*Unicode: U+0038 (8)*/
  {.w_px = 5,	.glyph_index = 175},	/*Unicode: U+0039 (9)*/
  {.w_px = 1,	.glyph_index = 182},	/*Unicode: U+003a (:)*/
  {.w_px = 2,	.glyph_index = 189},	/*Unicode: U+003b (;)*/
  {.w_px = 4,	.glyph_index = 196},	/*Unicode: U+003c (<)*/
  {.w_px = 4,	.glyph_index = 203},	/*Unicode: U+003d (=)*/
  {.w_px = 4,	.glyph_index = 210},	/*Unicode: U+003e (>)*/
  {.w_px = 4,	.glyph_index = 217},	/*Unicode: U+003f (?)*/
  {.w_px = 6,	.glyph_index = 224},	/*Unicode: U+0040 (@)*/
  {.w_px = 5,	.glyph_index = 231},	/*Unicode: U+0041 (A)*/
  {.w_px = 5,	.glyph_index = 238},	/*Unicode: U+0042 (B)*/
  {.w_px = 5,	.glyph_index = 245},	/*Unicode: U+0043 (C)*/
  {.w_px = 5,	.glyph_index = 252},	/*Unicode: U+0044 (D)*/
  {.w_px = 5,	.glyph_index = 259},	/*Unicode: U+0045 (E)*/
  {.w_px = 5,	.glyph_index = 266},	/*Unicode: U+0046 (F)*/
  {.w_px = 5,	.glyph_index = 273},	/*Unicode: U+0047 (G)*/
  {.w_px = 5,	.glyph_index = 280},	/*Unicode: U+0048 (H)*/
  {.w_px = 3,	.glyph_index = 287},	/*Unicode: U+0049 (I)*/
  {.w_px = 5,	.glyph_index = 294},	/*Unicode: U+004a (J)*/
  {.w_px = 5,	.glyph_index = 301},	/*Unicode: U+004b (K)*/
  {.w_px = 5,	.glyph_index = 308},	/*Unicode: U+004c (L)*/
  {.w_px = 5,	.glyph_index = 315},	/*Unicode: U+004d (M)*/
  {.w_px = 5,	.glyph_index = 322},	/*Unicode: U+004e (N)*/
  {.w_px = 5,	.glyph_index = 329},	/*Unicode: U+004f (O)*/
  {.w_px = 5,	.glyph_index = 336},	/*Unicode: U+0050 (P)*/
  {.w_px = 5,	.glyph_index = 343},	/*Unicode: U+0051 (Q)*/
  {.w_px = 5,	.glyph_index = 350},	/*Unicode: U+0052 (R)*/
  {.w_px = 5,	.glyph_index = 357},	/*Unicode: U+0053 (S)*/
  {.w_px = 4,	.glyph_index = 364},	/*Unicode: U+0054 (T)*/
  {.w_px = 5,	.glyph_index = 371},	/*Unicode: U+0055 (U)*/
  {.w_px = 5,	.glyph_index = 378},	/*Unicode: U+0056 (V)*/
  {.w_px = 5,	.glyph_index = 385},	/*Unicode: U+0057 (W)*/
  {.w_px = 5,	.glyph_index = 392},	/*Unicode: U+0058 (X)*/
  {.w_px = 5,	.glyph_index = 399},	/*Unicode: U+0059 (Y)*/
  {.w_px = 5,	.glyph_index = 406},	/*Unicode: U+005a (Z)*/
  {.w_px = 2,	.glyph_index = 413},	/*Unicode: U+005b ([)*/
  {.w_px = 3,	.glyph_index = 420},	/*Unicode: U+005c (\)*/
  {.w_px = 2,	.glyph_index = 427},	/*Unicode: U+005d (])*/
  {.w_px = 5,	.glyph_index = 434},	/*Unicode: U+005e (^)*/
  {.w_px = 4,	.glyph_index = 441},	/*Unicode: U+005f (_)*/
  {.w_px = 2,	.glyph_index = 448},	/*Unicode: U+0060 (`)*/
  {.w_px = 5,	.glyph_index = 455},	/*Unicode: U+0061 (a)*/
  {.w_px = 5,	.glyph_index = 462},	/*Unicode: U+0062 (b)*/
  {.w_px = 5,	.glyph_index = 469},	/*Unicode: U+0063 (c)*/
  {.w_px = 5,	.glyph_index = 476},	/*Unicode: U+0064 (d)*/
  {.w_px = 5,	.glyph_index = 483},	/*Unicode: U+0065 (e)*/
  {.w_px = 4,	.glyph_index = 490},	/*Unicode: U+0066 (f)*/
  {.w_px = 5,	.glyph_index = 497},	/*Unicode: U+0067 (g)*/
  {.w_px = 5,	.glyph_index = 504},	/*Unicode: U+0068 (h)*/
  {.w_px = 3,	.glyph_index = 511},	/*Unicode: U+0069 (i)*/
  {.w_px = 4,	.glyph_index = 518},	/*Unicode: U+006a (j)*/
  {.w_px = 4,	.glyph_index = 525},	/*Unicode: U+006b (k)*/
  {.w_px = 3,	.glyph_index = 532},	/*Unicode: U+006c (l)*/
  {.w_px = 5,	.glyph_index = 539},	/*Unicode: U+006d (m)*/
  {.w_px = 5,	.glyph_index = 546},	/*Unicode: U+006e (n)*/
  {.w_px = 5,	.glyph_index = 553},	/*Unicode: U+006f (o)*/
  {.w_px = 5,	.glyph_index = 560},	/*Unicode: U+0070 (p)*/
  {.w_px = 5,	.glyph_index = 567},	/*Unicode: U+0071 (q)*/
  {.w_px = 5,	.glyph_index = 574},	/*Unicode: U+0072 (r)*/
  {.w_px = 5,	.glyph_index = 581},	/*Unicode: U+0073 (s)*/
  {.w_px = 4,	.glyph_index = 588},	/*Unicode: U+0074 (t)*/
  {.w_px = 5,	.glyph_index = 595},	/*Unicode: U+0075 (u)*/
  {.w_px = 5,	.glyph_index = 602},	/*Unicode: U+0076 (v)*/
  {.w_px = 5,	.glyph_index = 609},	/*Unicode: U+0077 (w)*/
  {.w_px = 5,	.glyph_index = 616},	/*Unicode: U+0078 (x)*/
  {.w_px = 5,	.glyph_index = 623},	/*Unicode: U+0079 (y)*/
  {.w_px = 4,	.glyph_index = 630},	/*Unicode: U+007a (z)*/
  {.w_px = 3,	.glyph_index = 637},	/*Unicode: U+007b ({)*/
  {.w_px = 1,	.glyph_index = 644},	/*Unicode: U+007c (|)*/
  {.w_px = 3,	.glyph_index = 651},	/*Unicode: U+007d (})*/
  {.w_px = 5,	.glyph_index = 658},	/*Unicode: U+007e (~)*/
  {.w_px = 7,	.glyph_index = 665},	/*Unicode: U+007f ()*/
};

lv_font_t lv_font_pixelmix_7 = 
{
    .unicode_first = 32,	/*First Unicode letter in this font*/
    .unicode_last = 127,	/*Last Unicode letter in this font*/
    .h_px = 7,				/*Font height in pixels*/
    .glyph_bitmap = pixelmix_7_glyph_bitmap,	/*Bitmap of glyphs*/
    .glyph_dsc = pixelmix_7_glyph_dsc,		/*Description of glyphs*/
    .glyph_cnt = 96,			/*Number of glyphs in the font*/
    .unicode_list = NULL,	/*Every character in the font from 'unicode_first' to 'unicode_last'*/
    .get_bitmap = lv_font_get_bitmap_continuous,	/*Function pointer to get glyph's bitmap*/
    .get_width = lv_font_get_width_continuous,	/*Function pointer to get glyph's width*/
    .bpp = 1,				/*Bit per pixel*/
    .monospace = 0,				/*Fix width (0: if not used)*/
    .next_page = NULL,		/*Pointer to a font extension*/
};

#endif
