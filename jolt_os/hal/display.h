/**
 * @file display.h
 * @brief High-level commands to interact with display
 * @author Brian Pugh
 */

#ifndef JOLT_HAL_DISPLAY_H__
#define JOLT_HAL_DISPLAY_H__

#include "hal/lv_drivers/display/SSD1306.h"

#define DISPLAY_BRIGHTNESS_LEVELS 6

enum jolt_display_type { JOLT_DISPLAY_SSD1306 };
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
void jolt_display_init();

/**
 * @brief Gets the current screen brightness level
 *
 * level is an index into the brightness configuration arrays [0, DISPLAY_BRIGHTNESS_LEVELS)
 *
 * return current display brightness level
 */
uint8_t jolt_display_get_brightness();

/**
 * @brief Saves the provided screen brightness level to NVS
 * @param[in] index into the brightness array to save
 */
void jolt_display_save_brightness( uint8_t level );

/**
 * @brief Sets the screen brightness to the provided level ( up to DISPLAY_BRIGHTNESS_LEVELS )
 * @param[in] index into the brightness array to set
 */
void jolt_display_set_brightness( uint8_t level );

/**
 * @brief print the display buffer to stdout in a pretty format.
 */
void jolt_display_print();

/**
 * @brief Copy the current display to an output buffer.
 * @return Copy of the current screen buffer.
 */
display_data_t *jolt_display_copy();

/**
 * @brief Free a `display_data_t` object.
 */
void jolt_display_free( display_data_t *d );

/**
 * @brief Dump the display buffer to stdout as a data array.
 */
void jolt_display_dump();
#endif
