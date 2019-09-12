/**
 * @file display.h
 * @brief High-level commands to interact with display
 * @author Brian Pugh
 */

#ifndef JOLT_HAL_DISPLAY_H__
#define JOLT_HAL_DISPLAY_H__

#include "hal/lv_drivers/display/SSD1306.h"

#define DISPLAY_BRIGHTNESS_LEVELS 6

/**
 * @brief Configure LVGL display driver and hardware.
 *
 * Will reset device on failure.
 */
void display_init();

/**
 * @brief Gets the current screen brightness level 
 *
 * level is an index into the brightness configuration arrays [0, DISPLAY_BRIGHTNESS_LEVELS)
 *
 * return current display brightness level
 */
uint8_t get_display_brightness();

/**
 * @brief Saves the provided screen brightness level to NVS 
 * @param[in] index into the brightness array to save
 */
void save_display_brightness(uint8_t level);

/**
 * @brief Sets the screen brightness to the provided level ( up to DISPLAY_BRIGHTNESS_LEVELS ) 
 * @param[in] index into the brightness array to set
 */
void set_display_brightness(uint8_t level);

/**
 * @brief get memory chunk being used for display buffer
 * @return display buffer
 */
uint8_t *get_display_buf();

/**
 * @brief print the display buffer to stdout
 */
void print_display_buf();

#endif
