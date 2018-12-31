#ifndef JOLT_HAL_DISPLAY_H__
#define JOLT_HAL_DISPLAY_H__

#include "hal/lv_drivers/display/ssd1306.h"

extern ssd1306_t disp_hal;

void display_init();
uint8_t get_display_brightness();
void save_display_brightness(uint8_t brightness);

#endif
