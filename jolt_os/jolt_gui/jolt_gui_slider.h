/**
 * @file jolt_gui_slider.h
 * @brief Slider objects
 * @author Brian Pugh
 */

#ifndef __JOLT_GUI_SLIDER_H__
#define __JOLT_GUI_SLIDER_H__

#include "jolt_gui.h"
#include "lvgl/lvgl.h"

#ifndef CONFIG_JOLT_GUI_SLIDER_W
    #define CONFIG_JOLT_GUI_SLIDER_W 100
#endif
#ifndef CONFIG_JOLT_GUI_SLIDER_H
    #define CONFIG_JOLT_GUI_SLIDER_H 15
#endif
#ifndef CONFIG_JOLT_GUI_SLIDER_ANIM_MS
    #define CONFIG_JOLT_GUI_SLIDER_ANIM_MS 100
#endif

/**
 * @brief Get the slider object from a slider screen
 * @param[in] parent slider screen
 * @return lv_slider object
 */
jolt_gui_obj_t *jolt_gui_scr_slider_get_slider( jolt_gui_obj_t *parent );

/**
 * @brief Create a slider screen
 * @param[in] title title-bar string
 * @param[in] text string to diplsay above the slider
 * @param[in] cb callback to execute when slider changes position
 * @return slider screen
 */
jolt_gui_obj_t *jolt_gui_scr_slider_create( const char *title, const char *text, jolt_gui_event_cb_t cb );

/**
 * @brief Set the current slider value of a slider screen
 * @param[in,out] scr slider screen
 * @param value new slider value/position.
 */
void jolt_gui_scr_slider_set_value( jolt_gui_obj_t *scr, int16_t value );

/**
 * @brief Get the current slider value
 * @param[in] scr slider screen
 * @return current slider value
 */
int16_t jolt_gui_scr_slider_get_value( jolt_gui_obj_t *scr );

/**
 * @brief Set the slider screen's subtitle
 * @param[in,out] scr slider screen
 * @param[in] text new subtitle NULL-terminated string
 */
void jolt_gui_scr_slider_set_label( jolt_gui_obj_t *scr, const char *text );

/**
 * @brief Set the lower and upper limits of a slider screen
 * @param[in,out] scr slider screen
 * @param[in] min slider minimum value
 * @param[in] max slider maximum value
 */
void jolt_gui_scr_slider_set_range( jolt_gui_obj_t *scr, int16_t min, int16_t max );

#endif
