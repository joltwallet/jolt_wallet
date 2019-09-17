/**
 * @file jolt_gui_scroll.h
 * @brief Generic scrollable screens
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_SCROLL_H__
#define JOLT_GUI_SCROLL_H__

#include "jolt_gui_qr.h"

/**
 * @brief Create a scrollable screen
 *
 * This basically creates an lv_page in a screen (active object) that additional
 * elements can be added to.
 *
 * @param[in] title title-bar string
 * @return scroll screen
 */
jolt_gui_obj_t *jolt_gui_scr_scroll_create( const char *title );

/**
 * @brief Add text to a scroll screen
 *
 * Text is internally allocated and copied by LVGL. Text does not need to persist.
 *
 * @param[in,out] scr scroll screen
 * @param[in] text NULL-terminated string to display
 * @return created lv_label object
 */
jolt_gui_obj_t *jolt_gui_scr_scroll_add_text( jolt_gui_obj_t *scr, const char *text );

/**
 * @brief Add monospace text to a scroll screen
 *
 * Text is internally allocated and copied by LVGL. Text does not need to persist.
 *
 * @param[in,out] scr scroll screen
 * @param[in] text NULL-terminated string to display
 * @return created lv_label object
 */
jolt_gui_obj_t *jolt_gui_scr_scroll_add_monospace_text( jolt_gui_obj_t *scr, const char *text );

/**
 * @brief Get the lv_page object from a scroll screen
 * @param[in] scr scroll screen
 * @return lv_page object
 */
jolt_gui_obj_t *jolt_gui_scr_scroll_get_page( jolt_gui_obj_t *scr );

#endif
