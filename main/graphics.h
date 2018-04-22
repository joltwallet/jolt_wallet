#ifndef __NANORAY_GRAPHICS_H__
#define __NANORAY_GRAPHICS_H__

#include "u8g2.h"

#define GRAPHIC_NANO_LOGO_SMALL_H 22
#define GRAPHIC_NANO_LOGO_SMALL_W 50
extern const unsigned char graphic_nano_logo_small[];

#define GRAPHIC_NANO_RAY_H 22
#define GRAPHIC_NANO_RAY_W 62
extern const unsigned char graphic_nano_ray[];

void boot_splash(u8g2_t *u8g2);

#endif
