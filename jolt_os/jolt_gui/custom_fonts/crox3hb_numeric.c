
#include "lvgl/src/lv_misc/lv_font.h"

#if LV_USE_FONT_CROX3HB_NUMERIC != 0  /*Can be enabled in lv_conf.h*/

static const uint8_t crox3hb_glyph_bitmap[] =
{

/*Unicode: U+002b ( plus ) , Width: 8 */
0x00,  //........
0x00,  //........
0x00,  //........
0x1F,  //...%%...
0x1F,  //...%%...
0x1F,  //...%%...
0xFF,  //%%%%%%%%
0x1F,  //...%%...
0x1F,  //...%%...
0x1F,  //...%%...
0x00,  //........
0x00,  //........


/*Unicode: U+002d ( minus ) , Width: 6 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0xFC,  //%%%%%%..
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+002e ( period ) , Width: 3 */
0x00,  //...
0x00,  //...
0x00,  //...
0x00,  //...
0x00,  //...
0x00,  //...
0x00,  //...
0x00,  //...
0x00,  //...
0xE0,  //%%%
0xE0,  //%%%
0xE0,  //%%%


/*Unicode: U+0030 ( zero ) , Width: 8 */
0x3C,  //..%%%%..
0x66,  //.%%..%%.
0x66,  //.%%..%%.
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0x66,  //.%%..%%.
0x66,  //.%%..%%.
0x3C,  //..%%%%..


/*Unicode: U+0031 ( one ) , Width: 8 */
0x0C,  //....%%..
0x3C,  //..%%%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..


/*Unicode: U+0032 ( two ) , Width: 8 */
0x3C,  //..%%%%..
0x66,  //.%%..%%.
0xC3,  //%%....%%
0xC3,  //%%....%%
0x03,  //......%%
0x06,  //.....%%.
0x0C,  //....%%..
0x18,  //...%%...
0x30,  //..%%....
0x60,  //.%%.....
0xC0,  //%%......
0xFF,  //%%%%%%%%


/*Unicode: U+0033 ( three ) , Width: 8 */
0x3C,  //..%%%%..
0x66,  //.%%..%%.
0xC3,  //%%....%%
0xC3,  //%%....%%
0x06,  //.....%%.
0x3C,  //..%%%%..
0x06,  //.....%%.
0x03,  //......%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0x66,  //.%%..%%.
0x3C,  //..%%%%..


/*Unicode: U+0034 ( four ) , Width: 8 */
0x06,  //.....%%.
0x0E,  //....%%%.
0x0E,  //....%%%.
0x1E,  //...%%%%.
0x36,  //..%%.%%.
0x36,  //..%%.%%.
0x66,  //.%%..%%.
0xC6,  //%%...%%.
0xFF,  //%%%%%%%%
0x06,  //.....%%.
0x06,  //.....%%.
0x06,  //.....%%.


/*Unicode: U+0035 ( five ) , Width: 8 */
0xFF,  //%%%%%%%%
0xC0,  //%%......
0xC0,  //%%......
0xC0,  //%%......
0xFC,  //%%%%%%..
0xE6,  //%%%..%%.
0xC3,  //%%....%%
0x03,  //......%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0x66,  //.%%..%%.
0x3C,  //..%%%%..


/*Unicode: U+0036 ( six ) , Width: 8 */
0x3C,  //..%%%%..
0x66,  //.%%..%%.
0xC3,  //%%....%%
0xC0,  //%%......
0xFC,  //%%%%%%..
0xE6,  //%%%..%%.
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0x66,  //.%%..%%.
0x3C,  //..%%%%..


/*Unicode: U+0037 ( seven ) , Width: 8 */
0xFF,  //%%%%%%%%
0x03,  //......%%
0x03,  //......%%
0x06,  //.....%%.
0x06,  //.....%%.
0x0C,  //....%%..
0x0C,  //....%%..
0x0C,  //....%%..
0x18,  //...%%...
0x18,  //...%%...
0x18,  //...%%...
0x18,  //...%%...


/*Unicode: U+0038 ( eight ) , Width: 8 */
0x3C,  //..%%%%..
0x66,  //.%%..%%.
0xC3,  //%%....%%
0xC3,  //%%....%%
0x66,  //.%%..%%.
0x3C,  //..%%%%..
0x66,  //.%%..%%.
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0x66,  //.%%..%%.
0x3C,  //..%%%%..



/*Unicode: U+0039 ( nine ) , Width: 8 */
0x3C,  //..%%%%..
0x66,  //.%%..%%.
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0xC3,  //%%....%%
0x67,  //.%%..%%%
0x3F,  //..%%%%%%
0x03,  //......%%
0xC3,  //%%....%%
0x66,  //.%%..%%.
0x3C,  //..%%%%..

};

/*Store the glyph descriptions*/
static const lv_font_glyph_dsc_t crox3hb_glyph_dsc[] =
{
{.w_px = 8, .glyph_index = 0}, /*Unicode: U+002b ( )*/
{.w_px = 6, .glyph_index = 12}, /*Unicode: U+002d ( )*/
{.w_px = 3, .glyph_index = 24}, /*Unicode: U+002e ( )*/
{.w_px = 8, .glyph_index = 36}, /*Unicode: U+0030 ( )*/
{.w_px = 8, .glyph_index = 48}, /*Unicode: U+0031 ( )*/
{.w_px = 8, .glyph_index = 60}, /*Unicode: U+0032 ( )*/
{.w_px = 8, .glyph_index = 72}, /*Unicode: U+0033 ( )*/
{.w_px = 8, .glyph_index = 84}, /*Unicode: U+0034 ( )*/
{.w_px = 8, .glyph_index = 96}, /*Unicode: U+0035 ( )*/
{.w_px = 8, .glyph_index = 108}, /*Unicode: U+0036 ( )*/
{.w_px = 8, .glyph_index = 120}, /*Unicode: U+0037 ( )*/
{.w_px = 8, .glyph_index = 132}, /*Unicode: U+0038 ( )*/
{.w_px = 8, .glyph_index = 144}, /*Unicode: U+0039 ( )*/
};

static const uint32_t crox3hb_unicode_list[] = {
    0x2b,
    0x2d,
    0x2e,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
};

lv_font_t lv_font_crox3hb_numeric =
{
    .unicode_first = 0x2B,	/*First Unicode letter in this font*/
    .unicode_last = 0x39,	/*Last Unicode letter in this font*/
    .h_px = 12,				/*Font height in pixels*/
    .glyph_bitmap = crox3hb_glyph_bitmap,	/*Bitmap of glyphs*/
    .glyph_dsc = crox3hb_glyph_dsc,		/*Description of glyphs*/
    .unicode_list = crox3hb_unicode_list, /*List of unicode characters*/
    .get_bitmap = lv_font_get_bitmap_sparse,	/*Function pointer to get glyph's bitmap*/
    .get_width = lv_font_get_width_sparse,	/*Function pointer to get glyph's width*/
    .bpp = 1,				/*Bit per pixel*/
    .monospace = 0,				/*Fix width (0: if not used)*/
    .next_page = NULL,		/*Pointer to a font extension*/
    .glyph_cnt = 13,
};

#endif
