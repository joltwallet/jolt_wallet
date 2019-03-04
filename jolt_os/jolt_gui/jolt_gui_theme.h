/**
 * @file jolt_gui_theme.h
 * @brief LVGL theme specific for Jolt
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_THEME_H__
#define JOLT_GUI_THEME_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl/lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the jolt theme
 * @param[in] hue [0..360] hue value from HSV color space to define the theme's base color. Unused.
 * @param[in] font pointer to a font (NULL to use the default).
 * @return pointer to the initialized theme
 */
lv_theme_t * jolt_gui_theme_init(uint16_t hue, const lv_font_t *font);

/**
 * Get a pointer to the theme
 * @return pointer to the theme
 */
lv_theme_t * jolt_gui_theme_get(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
