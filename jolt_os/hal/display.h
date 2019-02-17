#ifndef JOLT_HAL_DISPLAY_H__
#define JOLT_HAL_DISPLAY_H__

#include "hal/lv_drivers/display/SSD1306.h"

#define DISPLAY_BRIGHTNESS_LEVELS 6

void display_init();

/* Gets the current screen brightness level */
uint8_t get_display_brightness();

/* Saves the provided screen brightness level to NVS */
void save_display_brightness(uint8_t level);

/* Sets the screen brightness to the provided level ( up to DISPLAY_BRIGHTNESS_LEVELS ) */
void set_display_brightness(uint8_t level);

#endif
