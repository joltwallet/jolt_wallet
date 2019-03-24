/**
 * @file jolt_gui_text.h
 * @brief alias for creating a scroll screen with text
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_TEXT_H__
#define JOLT_GUI_TEXT_H__

/**
 * @brief Display Text; Pressing any button returns to previous screen 
 * @param[in] title title-bar string
 * @param[in] body Text to display
 * @return scroll screen object containing text
 */
lv_obj_t *jolt_gui_scr_text_create(const char *title, const char *body);

#endif
