/**
 * @file jolt_gui_debug.h
 * @brief Functions to aid in LVGL GUI debugging.
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_DEBUG_H__
#define JOLT_GUI_DEBUG_H__

/**
 * @brief Prints out debug information about an object 
 *
 * Prints the following information:
 *     * Object pointer
 *     * Object type
 *     * Object size in bytes
 *     * Object's parent pointer
 *     * Object's parent type
 *
 * @param[in] obj LVGL object
 */
void jolt_gui_debug_obj_print(jolt_gui_obj_t *obj);

/**
 * @brief Gets the string equivalent of an event
 * @param event event to translate to a string
 * @return pointer to a constant null-terminated string
 */
const char *jolt_gui_event_to_str( jolt_gui_event_t event );

#endif
