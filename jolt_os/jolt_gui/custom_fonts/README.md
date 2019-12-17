#Jolt Custom Fonts

Fonts were modified with FontForge and then translated into C code via the 
[online lvgl font converter.](https://littlevgl.com/ttf-font-to-c-array).


## pixelmix

General system font.

###Font Modifications

Latin Supplement Expansion. Some glyphs have minor tweaks.

###Conversion Settings

* Input: `pixelmix.woff`
* Name: pixelmix
* Size: 8
* Bpp: 1
* Range: 0x20-0xFF

After conversion, manually change:

* `line_height` to `8`.
* `base_line` to `1`.


## jolt_symbols

Statusbar symbols like battery, wifi, etc.

###Conversion Settings

* Input: `jolt_symbols.ttf`
* Size: 8
* Bpp: 1
* Range: 0xF000-0xF00D

After conversion, manually change:

* `line_height` to `9`.


## unscii_8

Monospace font used for addresses and other data.

[unscii_8 source](http://pelulamu.net/unscii/).

###Font Modifications

* Left Bearing was reduced to 0; right bearing was reduced to 8.

###Conversion Settings

* Input: `unscii_8.woff`
* Size: 16
* Bpp: 1
* Range: 0x20-0x7F


## modenine_20

Just the numbers are used for PIN/digit entry.

###Font Modifications

Removed left and right bearings.

### Conversion Settings

* Input: `ModeNine.woff`
* Name: modenine_20
* Size: 20
* Bpp: 1
* Range: 0x2B-0x39

After conversion, manually change:

* `line_height` to `14`.
* `base_line` to `0`.

## modenine_40

Removed left and right bearings.

###Font Modifications

* Set Left and Right Bearing to 0.

### Conversion Settings

* Input: `ModeNine.woff`
* Name: modenine_40
* Size: 40
* Bpp: 1
* Range: 0x30-0x39

