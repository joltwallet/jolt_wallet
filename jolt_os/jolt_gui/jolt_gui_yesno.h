/**
 *
 * @bugs
 *     * If you do not delete the yes/no screen in the callback, the `no`
 *       callback will execute with event `jolt_gui_event.short_clicked`.
 */

#ifndef JOLT_GUI_YESNO_H__
#define JOLT_GUI_YESNO_H__

#include "lvgl/lvgl.h"

/**
 * @brief Create a yes/no screen
 * @param[in] title title-bar string
 * @param[in] yes_cb Callback for "yes" selection
 * @param[in] no_cb Callback for "no" selection or pressing the back button.
 *            If `NULL`, defaults to deleting the screen.
 */
jolt_gui_obj_t *jolt_gui_scr_yesno_create( const char *title, jolt_gui_event_cb_t yes_cb, jolt_gui_event_cb_t no_cb );

#endif
