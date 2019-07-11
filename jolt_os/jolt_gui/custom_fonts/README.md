#Jolt Custom Fonts

Fonts were modified with FontForge and then translated into C code via the 
[online lvgl font converter.](https://littlevgl.com/ttf-font-to-c-array).

## pixelmix

###Font Modifications
Latin Supplement Expansion. Some glyphs have minor tweaks.

###Conversion Settings
* Size: 8
* Bpp: 1
* Range: 0x20-0x7F

Also include `jolt_symbols` with the following setting:
* Size: 8
* Bpp: 1
* Range: 0xF000-0xF00D

After conversion, manually change the line_height from `9` to `8` at the bottom
of the `pixelmix.c` file.

## unscii_8

[unscii_8 source](http://pelulamu.net/unscii/).

###Font Modifications

* Left Bearing was reduced to 0; right bearing was reduced to 8.

###Conversion Settings

* Size: 16
* Bpp: 1
* Range: 0x20-0x7F
