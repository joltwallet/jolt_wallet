/**
 * @file display.h
 * @brief High-level commands to interact with display
 * @author Brian Pugh
 */

#ifndef JOLT_HAL_DISPLAY_H__
#define JOLT_HAL_DISPLAY_H__

#include "hal/lv_drivers/display/SSD1306.h"
#include "lvgl/lvgl.h"

#define JOLT_DISPLAY_BRIGHTNESS_LEVELS 6

/* For SSD1306 */
#define JOLT_DISPLAY_BUF_SIZE ( LV_HOR_RES_MAX * LV_VER_RES_MAX / 8 )

/**
 * @brief Encoding method for copying/printing the display buffer.
 */
enum {
    JOLT_DISPLAY_DUMP_ENCODING_NONE = 0,
    JOLT_DISPLAY_DUMP_ENCODING_RLE,
};
typedef uint8_t jolt_display_dump_encoding_t;

enum { JOLT_DISPLAY_TYPE_SSD1306 = 0 };
typedef uint8_t jolt_display_type_t;

typedef struct jolt_display_t {
    jolt_display_type_t type;
    jolt_display_dump_encoding_t encoding;
    uint32_t len;  /**< Length of data */
    uint8_t *data; /**< Data encoding via `encoding` */
} jolt_display_t;

/**
 * @brief Configure LVGL display driver and hardware.
 *
 * Will reset device on failure.
 */
void jolt_display_init();

/**
 * @brief Gets the current screen brightness level
 *
 * level is an index into the brightness configuration arrays [0, JOLT_DISPLAY_BRIGHTNESS_LEVELS)
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
 * @brief Sets the screen brightness to the provided level ( up to JOLT_DISPLAY_BRIGHTNESS_LEVELS )
 * @param[in] index into the brightness array to set
 */
void jolt_display_set_brightness( uint8_t level );

/**
 * @brief print the display buffer to stdout in a pretty format.
 * @param[in] user_buf Copy of display buffer (see `jolt_display_copy`). If NULL, the
 *            current screen is printed.
 * @param[in] encoding Method user_buf is encoded.
 */
void jolt_display_print( const jolt_display_t *disp );

/**
 * @brief Copy the current display to an output buffer.
 * @param[in] encoding Method to encode returned buffer.
 * @param[in,out] copy Output to save copy of display data. Will encode the copied
 * data via the value in `copy->encoding`.
 * @return `true` on success.
 */
bool jolt_display_copy( jolt_display_t *copy );

/**
 * @brief Dump the display buffer to stdout as a data array.
 * @param[in] disp Copy of display buffer (see `jolt_display_copy`). If NULL, the
 *            current screen is dumped with no encoding.
 * @param[in] encoding Method user_buf is encoded.
 */
void jolt_display_dump( const jolt_display_t *disp );

/**
 * @brief Free allocated parts of a `jolt_display_t` object.
 * @param[in] disp
 */
void jolt_display_free( jolt_display_t *disp );

#endif
