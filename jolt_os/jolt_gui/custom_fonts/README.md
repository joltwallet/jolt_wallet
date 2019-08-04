#Jolt Custom Fonts

Fonts were modified with FontForge and then translated into C code via the 
[online lvgl font converter.](https://littlevgl.com/ttf-font-to-c-array).

## pixelmix

General system font.

###Font Modifications

Latin Supplement Expansion. Some glyphs have minor tweaks.

###Conversion Settings

* Size: 8
* Bpp: 1
* Range: 0x20-0xFF

After conversion, manually change:

* `line_height` to `8`.

## jolt_symbols

Statusbar symbols like battery, wifi, etc.

###Conversion Settings

* Size: 8
* Bpp: 1
* Range: 0xF000-0xF00D

After conversion, manually change:

* `line_height` to `9`.
* `base_line` to `1`.

## unscii_8

Monospace font.

[unscii_8 source](http://pelulamu.net/unscii/).

###Font Modifications

* Left Bearing was reduced to 0; right bearing was reduced to 8.

###Conversion Settings

* Size: 16
* Bpp: 1
* Range: 0x20-0x7F
