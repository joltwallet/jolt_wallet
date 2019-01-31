#ifndef JOLT_HAL_DISPLAY_H__
#define JOLT_HAL_DISPLAY_H__

#include "hal/lv_drivers/display/ssd1306.h"

#define DISPLAY_BRIGHTNESS_LEVELS 6

void display_init();

uint8_t get_display_brightness();
void save_display_brightness(uint8_t level);

void set_display_brightness(uint8_t level);

#endif
