#Jolt Custom Fonts

Fonts were modified with FontForge and then translated into C code via the 
[online lvgl font converter.](https://littlevgl.com/ttf-font-to-c-array).


## pixelmix

General system font.

###Font Modifications

Latin Supplement Expansion. Some glyphs have minor tweaks.

###Conversion Settings

* Input: `pixelmix.woff`
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

Monospace font used for addresses and other data.

[unscii_8 source](http://pelulamu.net/unscii/).

###Font Modifications

* Left Bearing was reduced to 0; right bearing was reduced to 8.

###Conversion Settings

* Input: `unscii_8.woff`
* Size: 16
* Bpp: 1
* Range: 0x20-0x7F


## dejavu_16

Just the numbers are used for PIN/digit entry.

###Font Modifications

[TODO] Cleaned up some glyphs for pixel perfectness at this size.

### Conversion Settings

* Input: `dejavu-fonts-ttf-2.37/ttf/DejaVuSansMono-Bold.ttf`
* Size: 16
* Bpp: 1
* Range: 0x2B-0x39

After conversion, manually change:

* `line_height` to `12`.
* `base_line` to `0`.

## dejavu_32

Just the numbers are used for bluetooth pairing.

[TODO] Cleaned up some glyphs for pixel perfectness at this size.

###Font Modifications

Originated from `dejavu-fonts-ttf-2.37/ttf/DejaVuSans.ttf`

* Set Left and Right Bearing to 0.

### Conversion Settings

* Input: `DejaVuSans09NoBearing.woff`
* Size: 32
* Bpp: 1
* Range: 0x30-0x39

