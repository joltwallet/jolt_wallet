/**
 * @file display.h
 * @brief High-level commands to interact with display
 * @author Brian Pugh
 */

#ifndef JOLT_HAL_DISPLAY_H__
#define JOLT_HAL_DISPLAY_H__

#include "hal/lv_drivers/display/SSD1306.h"

#define DISPLAY_BRIGHTNESS_LEVELS 6

enum jolt_display_type{
    JOLT_DISPLAY_SSD1306
};
typedef uint8_t jolt_display_type_t; 

typedef struct {
    jolt_display_type_t type;
    uint8_t *data;
} display_data_t;

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
void save_display_brightness( uint8_t level );

/**
 * @brief Sets the screen brightness to the provided level ( up to DISPLAY_BRIGHTNESS_LEVELS )
 * @param[in] index into the brightness array to set
 */
void set_display_brightness( uint8_t level );

/**
 * @brief print the display buffer to stdout
 */
void print_display_buf();

/**
 * @brief Copy the current display to an output buffer.
 * @return Copy of the current screen buffer.
 */
display_data_t *jolt_copy_display_buf();

#endif
