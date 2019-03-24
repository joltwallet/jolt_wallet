/**
 * @file jolt_gui_debug.h
 * @brief Functions to aid in LVGL GUI debugging.
 * @author Brian Pugh
 */

#if JOLT_GUI_DEBUG_FUNCTIONS
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
void jolt_gui_debug_obj_print(lv_obj_t *obj);

#endif
#endif
