
#include "../lv_misc/lv_font.h"

#if USE_LV_FONT_SCIENTIFICA_11 != 0  /*Can be enabled in lv_conf.h*/

static const uint8_t scientifica_11_glyph_bitmap[] =
{
/*Unicode: U+0020 ( space ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+0021 ( exclam ) , Width: 5 */
0x00,  //........
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x00,  //........
0x20,  //..%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0022 ( quotedbl ) , Width: 5 */
0x00,  //........
0x50,  //.%.%....
0x50,  //.%.%....
0x50,  //.%.%....
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+0023 ( numbersign ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x50,  //.%.%....
0xF8,  //%%%%%...
0x50,  //.%.%....
0xF8,  //%%%%%...
0x50,  //.%.%....
0x00,  //........
0x00,  //........


/*Unicode: U+0024 ( dollar ) , Width: 5 */
0x20,  //..%.....
0x20,  //..%.....
0x70,  //.%%%....
0x40,  //.%......
0x70,  //.%%%....
0x10,  //...%....
0x70,  //.%%%....
0x20,  //..%.....
0x20,  //..%.....
0x00,  //........


/*Unicode: U+0025 ( percent ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x90,  //%..%....
0x20,  //..%.....
0x40,  //.%......
0x90,  //%..%....
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+0026 ( ampersand ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x80,  //%.......
0x90,  //%..%....
0x78,  //.%%%%...
0x90,  //%..%....
0x90,  //%..%....
0x68,  //.%%.%...
0x00,  //........
0x00,  //........


/*Unicode: U+0027 ( quotesingle ) , Width: 5 */
0x00,  //........
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+0028 ( parenleft ) , Width: 5 */
0x30,  //..%%....
0x40,  //.%......
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x40,  //.%......
0x30,  //..%%....
0x00,  //........


/*Unicode: U+0029 ( parenright ) , Width: 5 */
0xC0,  //%%......
0x20,  //..%.....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x20,  //..%.....
0xC0,  //%%......
0x00,  //........


/*Unicode: U+002a ( asterisk ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x50,  //.%.%....
0x20,  //..%.....
0xF8,  //%%%%%...
0x20,  //..%.....
0x50,  //.%.%....
0x00,  //........
0x00,  //........


/*Unicode: U+002b ( plus ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x20,  //..%.....
0xF8,  //%%%%%...
0x20,  //..%.....
0x20,  //..%.....
0x00,  //........
0x00,  //........


/*Unicode: U+002c ( comma ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x20,  //..%.....
0x40,  //.%......
0x00,  //........


/*Unicode: U+002d ( hyphen ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0xF0,  //%%%%....
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+002e ( period ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x00,  //........
0x00,  //........


/*Unicode: U+002f ( slash ) , Width: 5 */
0x00,  //........
0x10,  //...%....
0x10,  //...%....
0x20,  //..%.....
0x20,  //..%.....
0x40,  //.%......
0x40,  //.%......
0x80,  //%.......
0x80,  //%.......
0x00,  //........


/*Unicode: U+0030 ( zero ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0xB0,  //%.%%....
0xD0,  //%%.%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0031 ( one ) , Width: 5 */
0x00,  //........
0x20,  //..%.....
0x60,  //.%%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0032 ( two ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x10,  //...%....
0x20,  //..%.....
0x40,  //.%......
0x80,  //%.......
0xF0,  //%%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0033 ( three ) , Width: 5 */
0x00,  //........
0xF0,  //%%%%....
0x10,  //...%....
0x20,  //..%.....
0x50,  //.%.%....
0x10,  //...%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0034 ( four ) , Width: 5 */
0x00,  //........
0x80,  //%.......
0x80,  //%.......
0xA0,  //%.%.....
0xA0,  //%.%.....
0xF0,  //%%%%....
0x20,  //..%.....
0x20,  //..%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0035 ( five ) , Width: 5 */
0x00,  //........
0xF0,  //%%%%....
0x80,  //%.......
0xE0,  //%%%.....
0x10,  //...%....
0x10,  //...%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0036 ( six ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x80,  //%.......
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0037 ( seven ) , Width: 5 */
0x00,  //........
0xF0,  //%%%%....
0x10,  //...%....
0x10,  //...%....
0x20,  //..%.....
0x20,  //..%.....
0x40,  //.%......
0x40,  //.%......
0x00,  //........
0x00,  //........


/*Unicode: U+0038 ( eight ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0039 ( nine ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x10,  //...%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+003a ( colon ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+003b ( semicolon ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x20,  //..%.....
0x40,  //.%......
0x00,  //........


/*Unicode: U+003c ( less ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x20,  //..%.....
0x40,  //.%......
0x80,  //%.......
0x40,  //.%......
0x20,  //..%.....
0x00,  //........
0x00,  //........


/*Unicode: U+003d ( equal ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0xF0,  //%%%%....
0x00,  //........
0xF0,  //%%%%....
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+003e ( greater ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x40,  //.%......
0x20,  //..%.....
0x10,  //...%....
0x20,  //..%.....
0x40,  //.%......
0x00,  //........
0x00,  //........


/*Unicode: U+003f ( question ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x10,  //...%....
0x20,  //..%.....
0x40,  //.%......
0x00,  //........
0x40,  //.%......
0x00,  //........
0x00,  //........


/*Unicode: U+0040 ( at ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0xB0,  //%.%%....
0xB0,  //%.%%....
0xB0,  //%.%%....
0x80,  //%.......
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0041 ( A ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0xF0,  //%%%%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0042 ( B ) , Width: 5 */
0x00,  //........
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0xE0,  //%%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0043 ( C ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0044 ( D ) , Width: 5 */
0x00,  //........
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0xE0,  //%%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0045 ( E ) , Width: 5 */
0x00,  //........
0xF0,  //%%%%....
0x80,  //%.......
0x80,  //%.......
0xE0,  //%%%.....
0x80,  //%.......
0x80,  //%.......
0xF0,  //%%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0046 ( F ) , Width: 5 */
0x00,  //........
0xF0,  //%%%%....
0x80,  //%.......
0x80,  //%.......
0xE0,  //%%%.....
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x00,  //........
0x00,  //........


/*Unicode: U+0047 ( G ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x80,  //%.......
0xB0,  //%.%%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0048 ( H ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0xF0,  //%%%%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0049 ( I ) , Width: 5 */
0x00,  //........
0x70,  //.%%%....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+004a ( J ) , Width: 5 */
0x00,  //........
0x70,  //.%%%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x90,  //%..%....
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+004b ( K ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0xA0,  //%.%.....
0xC0,  //%%......
0xA0,  //%.%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+004c ( L ) , Width: 5 */
0x00,  //........
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0xF0,  //%%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+004d ( M ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0xF0,  //%%%%....
0xF0,  //%%%%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+004e ( N ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0xD0,  //%%.%....
0xD0,  //%%.%....
0xB0,  //%.%%....
0xB0,  //%.%%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+004f ( O ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0050 ( P ) , Width: 5 */
0x00,  //........
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0xE0,  //%%%.....
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x00,  //........
0x00,  //........


/*Unicode: U+0051 ( Q ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x20,  //..%.....
0x10,  //...%....


/*Unicode: U+0052 ( R ) , Width: 5 */
0x00,  //........
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0053 ( S ) , Width: 5 */
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x80,  //%.......
0x60,  //.%%.....
0x10,  //...%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0054 ( T ) , Width: 5 */
0x00,  //........
0xF8,  //%%%%%...
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0055 ( U ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0056 ( V ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0057 ( W ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0xF0,  //%%%%....
0xF0,  //%%%%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0058 ( X ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0059 ( Y ) , Width: 5 */
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x10,  //...%....
0x90,  //%..%....
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+005a ( Z ) , Width: 5 */
0x00,  //........
0xF0,  //%%%%....
0x10,  //...%....
0x20,  //..%.....
0x40,  //.%......
0x80,  //%.......
0x80,  //%.......
0xF0,  //%%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+005b ( bracketleft ) , Width: 5 */
0x70,  //.%%%....
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x70,  //.%%%....
0x00,  //........


/*Unicode: U+005c ( backslash ) , Width: 5 */
0x00,  //........
0x80,  //%.......
0x80,  //%.......
0x40,  //.%......
0x40,  //.%......
0x20,  //..%.....
0x20,  //..%.....
0x10,  //...%....
0x10,  //...%....
0x00,  //........


/*Unicode: U+005d ( bracketright ) , Width: 5 */
0x70,  //.%%%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x10,  //...%....
0x70,  //.%%%....
0x00,  //........


/*Unicode: U+005e ( asciicircum ) , Width: 5 */
0x00,  //........
0x20,  //..%.....
0x50,  //.%.%....
0x88,  //%...%...
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+005f ( underscore ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0xF0,  //%%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0060 ( grave ) , Width: 5 */
0x00,  //........
0x40,  //.%......
0x20,  //..%.....
0x10,  //...%....
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........


/*Unicode: U+0061 ( a ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x70,  //.%%%....
0x90,  //%..%....
0x90,  //%..%....
0xB0,  //%.%%....
0x50,  //.%.%....
0x00,  //........
0x00,  //........


/*Unicode: U+0062 ( b ) , Width: 5 */
0x00,  //........
0x80,  //%.......
0x80,  //%.......
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0xE0,  //%%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0063 ( c ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x80,  //%.......
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0064 ( d ) , Width: 5 */
0x00,  //........
0x10,  //...%....
0x10,  //...%....
0x70,  //.%%%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0065 ( e ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0xF0,  //%%%%....
0x80,  //%.......
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0066 ( f ) , Width: 5 */
0x00,  //........
0x30,  //..%%....
0x40,  //.%......
0x40,  //.%......
0xE0,  //%%%.....
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x00,  //........
0x00,  //........


/*Unicode: U+0067 ( g ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x70,  //.%%%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x10,  //...%....
0x60,  //.%%.....


/*Unicode: U+0068 ( h ) , Width: 5 */
0x00,  //........
0x80,  //%.......
0x80,  //%.......
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0069 ( i ) , Width: 5 */
0x00,  //........
0x40,  //.%......
0x00,  //........
0xC0,  //%%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x30,  //..%%....
0x00,  //........
0x00,  //........


/*Unicode: U+006a ( j ) , Width: 5 */
0x00,  //........
0x20,  //..%.....
0x00,  //........
0x60,  //.%%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0xC0,  //%%......


/*Unicode: U+006b ( k ) , Width: 5 */
0x00,  //........
0x80,  //%.......
0x80,  //%.......
0x90,  //%..%....
0xA0,  //%.%.....
0xC0,  //%%......
0xA0,  //%.%.....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+006c ( l ) , Width: 5 */
0x00,  //........
0xC0,  //%%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x30,  //..%%....
0x00,  //........
0x00,  //........


/*Unicode: U+006d ( m ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x90,  //%..%....
0xF0,  //%%%%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+006e ( n ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+006f ( o ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0070 ( p ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0xE0,  //%%%.....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0xE0,  //%%%.....
0x80,  //%.......
0x80,  //%.......


/*Unicode: U+0071 ( q ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x70,  //.%%%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x10,  //...%....
0x10,  //...%....


/*Unicode: U+0072 ( r ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0xE0,  //%%%.....
0x90,  //%..%....
0x80,  //%.......
0x80,  //%.......
0x80,  //%.......
0x00,  //........
0x00,  //........


/*Unicode: U+0073 ( s ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x70,  //.%%%....
0x80,  //%.......
0x60,  //.%%.....
0x10,  //...%....
0xE0,  //%%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0074 ( t ) , Width: 5 */
0x00,  //........
0x40,  //.%......
0x40,  //.%......
0xF0,  //%%%%....
0x40,  //.%......
0x40,  //.%......
0x40,  //.%......
0x30,  //..%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0075 ( u ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+0076 ( v ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x60,  //.%%.....
0x00,  //........
0x00,  //........


/*Unicode: U+0077 ( w ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0xF0,  //%%%%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0078 ( x ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x60,  //.%%.....
0x90,  //%..%....
0x90,  //%..%....
0x00,  //........
0x00,  //........


/*Unicode: U+0079 ( y ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x90,  //%..%....
0x70,  //.%%%....
0x10,  //...%....
0x60,  //.%%.....


/*Unicode: U+007a ( z ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0xF0,  //%%%%....
0x20,  //..%.....
0x40,  //.%......
0x80,  //%.......
0xF0,  //%%%%....
0x00,  //........
0x00,  //........


/*Unicode: U+007b ( braceleft ) , Width: 5 */
0x10,  //...%....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0xC0,  //%%......
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x10,  //...%....
0x00,  //........


/*Unicode: U+007c ( bar ) , Width: 5 */
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x00,  //........


/*Unicode: U+007d ( braceright ) , Width: 5 */
0x40,  //.%......
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x18,  //...%%...
0x20,  //..%.....
0x20,  //..%.....
0x20,  //..%.....
0x40,  //.%......
0x00,  //........


/*Unicode: U+007e ( asciitilde ) , Width: 5 */
0x00,  //........
0x00,  //........
0x00,  //........
0x00,  //........
0x40,  //.%......
0xA8,  //%.%.%...
0x10,  //...%....
0x00,  //........
0x00,  //........
0x00,  //........



};

/*Store the glyph descriptions*/
static const lv_font_glyph_dsc_t scientifica_11_glyph_dsc[] =
{

{.w_px = 5, .glyph_index = 0}, /*Unicode: U+0020 ( )*/

{.w_px = 5, .glyph_index = 10}, /*Unicode: U+0021 ( )*/

{.w_px = 5, .glyph_index = 20}, /*Unicode: U+0022 ( )*/

{.w_px = 5, .glyph_index = 30}, /*Unicode: U+0023 ( )*/

{.w_px = 5, .glyph_index = 40}, /*Unicode: U+0024 ( )*/

{.w_px = 5, .glyph_index = 50}, /*Unicode: U+0025 ( )*/

{.w_px = 5, .glyph_index = 60}, /*Unicode: U+0026 ( )*/

{.w_px = 5, .glyph_index = 70}, /*Unicode: U+0027 ( )*/

{.w_px = 5, .glyph_index = 80}, /*Unicode: U+0028 ( )*/

{.w_px = 5, .glyph_index = 90}, /*Unicode: U+0029 ( )*/

{.w_px = 5, .glyph_index = 100}, /*Unicode: U+002a ( )*/

{.w_px = 5, .glyph_index = 110}, /*Unicode: U+002b ( )*/

{.w_px = 5, .glyph_index = 120}, /*Unicode: U+002c ( )*/

{.w_px = 5, .glyph_index = 130}, /*Unicode: U+002d ( )*/

{.w_px = 5, .glyph_index = 140}, /*Unicode: U+002e ( )*/

{.w_px = 5, .glyph_index = 150}, /*Unicode: U+002f ( )*/

{.w_px = 5, .glyph_index = 160}, /*Unicode: U+0030 ( )*/

{.w_px = 5, .glyph_index = 170}, /*Unicode: U+0031 ( )*/

{.w_px = 5, .glyph_index = 180}, /*Unicode: U+0032 ( )*/

{.w_px = 5, .glyph_index = 190}, /*Unicode: U+0033 ( )*/

{.w_px = 5, .glyph_index = 200}, /*Unicode: U+0034 ( )*/

{.w_px = 5, .glyph_index = 210}, /*Unicode: U+0035 ( )*/

{.w_px = 5, .glyph_index = 220}, /*Unicode: U+0036 ( )*/

{.w_px = 5, .glyph_index = 230}, /*Unicode: U+0037 ( )*/

{.w_px = 5, .glyph_index = 240}, /*Unicode: U+0038 ( )*/

{.w_px = 5, .glyph_index = 250}, /*Unicode: U+0039 ( )*/

{.w_px = 5, .glyph_index = 260}, /*Unicode: U+003a ( )*/

{.w_px = 5, .glyph_index = 270}, /*Unicode: U+003b ( )*/

{.w_px = 5, .glyph_index = 280}, /*Unicode: U+003c ( )*/

{.w_px = 5, .glyph_index = 290}, /*Unicode: U+003d ( )*/

{.w_px = 5, .glyph_index = 300}, /*Unicode: U+003e ( )*/

{.w_px = 5, .glyph_index = 310}, /*Unicode: U+003f ( )*/

{.w_px = 5, .glyph_index = 320}, /*Unicode: U+0040 ( )*/

{.w_px = 5, .glyph_index = 330}, /*Unicode: U+0041 ( )*/

{.w_px = 5, .glyph_index = 340}, /*Unicode: U+0042 ( )*/

{.w_px = 5, .glyph_index = 350}, /*Unicode: U+0043 ( )*/

{.w_px = 5, .glyph_index = 360}, /*Unicode: U+0044 ( )*/

{.w_px = 5, .glyph_index = 370}, /*Unicode: U+0045 ( )*/

{.w_px = 5, .glyph_index = 380}, /*Unicode: U+0046 ( )*/

{.w_px = 5, .glyph_index = 390}, /*Unicode: U+0047 ( )*/

{.w_px = 5, .glyph_index = 400}, /*Unicode: U+0048 ( )*/

{.w_px = 5, .glyph_index = 410}, /*Unicode: U+0049 ( )*/

{.w_px = 5, .glyph_index = 420}, /*Unicode: U+004a ( )*/

{.w_px = 5, .glyph_index = 430}, /*Unicode: U+004b ( )*/

{.w_px = 5, .glyph_index = 440}, /*Unicode: U+004c ( )*/

{.w_px = 5, .glyph_index = 450}, /*Unicode: U+004d ( )*/

{.w_px = 5, .glyph_index = 460}, /*Unicode: U+004e ( )*/

{.w_px = 5, .glyph_index = 470}, /*Unicode: U+004f ( )*/

{.w_px = 5, .glyph_index = 480}, /*Unicode: U+0050 ( )*/

{.w_px = 5, .glyph_index = 490}, /*Unicode: U+0051 ( )*/

{.w_px = 5, .glyph_index = 500}, /*Unicode: U+0052 ( )*/

{.w_px = 5, .glyph_index = 510}, /*Unicode: U+0053 ( )*/

{.w_px = 5, .glyph_index = 520}, /*Unicode: U+0054 ( )*/

{.w_px = 5, .glyph_index = 530}, /*Unicode: U+0055 ( )*/

{.w_px = 5, .glyph_index = 540}, /*Unicode: U+0056 ( )*/

{.w_px = 5, .glyph_index = 550}, /*Unicode: U+0057 ( )*/

{.w_px = 5, .glyph_index = 560}, /*Unicode: U+0058 ( )*/

{.w_px = 5, .glyph_index = 570}, /*Unicode: U+0059 ( )*/

{.w_px = 5, .glyph_index = 580}, /*Unicode: U+005a ( )*/

{.w_px = 5, .glyph_index = 590}, /*Unicode: U+005b ( )*/

{.w_px = 5, .glyph_index = 600}, /*Unicode: U+005c ( )*/

{.w_px = 5, .glyph_index = 610}, /*Unicode: U+005d ( )*/

{.w_px = 5, .glyph_index = 620}, /*Unicode: U+005e ( )*/

{.w_px = 5, .glyph_index = 630}, /*Unicode: U+005f ( )*/

{.w_px = 5, .glyph_index = 640}, /*Unicode: U+0060 ( )*/

{.w_px = 5, .glyph_index = 650}, /*Unicode: U+0061 ( )*/

{.w_px = 5, .glyph_index = 660}, /*Unicode: U+0062 ( )*/

{.w_px = 5, .glyph_index = 670}, /*Unicode: U+0063 ( )*/

{.w_px = 5, .glyph_index = 680}, /*Unicode: U+0064 ( )*/

{.w_px = 5, .glyph_index = 690}, /*Unicode: U+0065 ( )*/

{.w_px = 5, .glyph_index = 700}, /*Unicode: U+0066 ( )*/

{.w_px = 5, .glyph_index = 710}, /*Unicode: U+0067 ( )*/

{.w_px = 5, .glyph_index = 720}, /*Unicode: U+0068 ( )*/

{.w_px = 5, .glyph_index = 730}, /*Unicode: U+0069 ( )*/

{.w_px = 5, .glyph_index = 740}, /*Unicode: U+006a ( )*/

{.w_px = 5, .glyph_index = 750}, /*Unicode: U+006b ( )*/

{.w_px = 5, .glyph_index = 760}, /*Unicode: U+006c ( )*/

{.w_px = 5, .glyph_index = 770}, /*Unicode: U+006d ( )*/

{.w_px = 5, .glyph_index = 780}, /*Unicode: U+006e ( )*/

{.w_px = 5, .glyph_index = 790}, /*Unicode: U+006f ( )*/

{.w_px = 5, .glyph_index = 800}, /*Unicode: U+0070 ( )*/

{.w_px = 5, .glyph_index = 810}, /*Unicode: U+0071 ( )*/

{.w_px = 5, .glyph_index = 820}, /*Unicode: U+0072 ( )*/

{.w_px = 5, .glyph_index = 830}, /*Unicode: U+0073 ( )*/

{.w_px = 5, .glyph_index = 840}, /*Unicode: U+0074 ( )*/

{.w_px = 5, .glyph_index = 850}, /*Unicode: U+0075 ( )*/

{.w_px = 5, .glyph_index = 860}, /*Unicode: U+0076 ( )*/

{.w_px = 5, .glyph_index = 870}, /*Unicode: U+0077 ( )*/

{.w_px = 5, .glyph_index = 880}, /*Unicode: U+0078 ( )*/

{.w_px = 5, .glyph_index = 890}, /*Unicode: U+0079 ( )*/

{.w_px = 5, .glyph_index = 900}, /*Unicode: U+007a ( )*/

{.w_px = 5, .glyph_index = 910}, /*Unicode: U+007b ( )*/

{.w_px = 5, .glyph_index = 920}, /*Unicode: U+007c ( )*/

{.w_px = 5, .glyph_index = 930}, /*Unicode: U+007d ( )*/

{.w_px = 5, .glyph_index = 940}, /*Unicode: U+007e ( )*/

};

lv_font_t scientifica_11 =
{
    .unicode_first = 32,	/*First Unicode letter in this font*/
    .unicode_last = 126,	/*Last Unicode letter in this font*/
    .h_px = 10,				/*Font height in pixels*/
    .glyph_bitmap = scientifica_11_glyph_bitmap,	/*Bitmap of glyphs*/
    .glyph_dsc = scientifica_11_glyph_dsc,		/*Description of glyphs*/
    .glyph_cnt = 95,			/*Number of glyphs in the font*/
    .unicode_list = NULL,	/*Every character in the font from 'unicode_first' to 'unicode_last'*/
    .get_bitmap = lv_font_get_bitmap_continuous,	/*Function pointer to get glyph's bitmap*/
    .get_width = lv_font_get_width_continuous,	/*Function pointer to get glyph's width*/
    .bpp = 1,				/*Bit per pixel*/
    .monospace = 0,				/*Fix width (0: if not used)*/
    .next_page = NULL,		/*Pointer to a font extension*/
};

#endif