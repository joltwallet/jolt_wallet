/**
 * @file jolt_gui_loading.h
 * @brief Loading Screens
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_LOADING_H__
#define JOLT_GUI_LOADING_H__

#include "jolt_gui/jolt_gui.h"

#ifndef CONFIG_JOLT_GUI_LOADINGBAR_W
    #define CONFIG_JOLT_GUI_LOADINGBAR_W 100
#endif
#ifndef CONFIG_JOLT_GUI_LOADINGBAR_H
    #define CONFIG_JOLT_GUI_LOADINGBAR_H 15
#endif
#ifndef CONFIG_JOLT_GUI_LOADINGBAR_ANIM_MS
    #define CONFIG_JOLT_GUI_LOADINGBAR_ANIM_MS 100
#endif

/**
 * @brief Creates a loading-bar screen
 *
 * Progress is started at 0 by default.
 *
 * @param[in] title Statusbar title string
 * @return loading-bar screen
 */
jolt_gui_obj_t *jolt_gui_scr_loadingbar_create( const char *title );


/**
 * @brief Update the loading-bar progress
 *
 * If `autoupdate` is enabled (see `jolt_gui_scr_loadingbar_autoupdate()`), then
 * the percentage will be updated if >= 0.
 *
 * @param[in,out] parent loading-bar screen
 * @param[in] title Statusbar title string. NULL to not change the current title.
 * @param[in] text Subtitle text displayed above the loading-bar. NULL to not change the current subtitle.
 * @param[in] percentage Current progress [0~100]. Set to a negative value to not update the progressbar.
 */
void jolt_gui_scr_loadingbar_update( jolt_gui_obj_t *parent, const char *title, const char *text, int8_t percentage );


/**
 * @brief Automatically updates the loading bar according to the value of progress [0~100]
 *
 * Will send an jolt_gui_event.value_changed to the active element's event_cb.
 * Will send an jolt_gui_event.apply to the active element's event_cb upon 100%
 *
 * @param[in,out] parent loading-bar screen
 * @return Pointer to current progress [0~100] which will be periodically read from. Writing to this value will update
 * the progress bar.
 */
int8_t *jolt_gui_scr_loadingbar_autoupdate( jolt_gui_obj_t *parent );


/**
 * @brief Cleanup the autoupdate task.
 *
 * This gets called internally when the parent is deleted. Usually end-user should never call this.
 *
 * @param[in,out] parent loading-bar screen
 */
void jolt_gui_scr_loadingbar_autoupdate_del( jolt_gui_obj_t *parent );


/**
 * @brief Get pointer to progress value
 * @param[in,out] obj Screen or bar object
 */
int8_t *jolt_gui_scr_loadingbar_progress_get( lv_obj_t *obj );


/**
 * @brief Creates a spinning arc preloading screen.
 *
 * Typically used for tasks that have unknown progress to show the user.
 *
 * @param[in] title Statusbar title string. NULL for an empty title
 * @param[in] text subtitle text displayed above the spinning arc. NULL for an empty subtitle.

 * @return preloading screen
 */
jolt_gui_obj_t *jolt_gui_scr_preloading_create( const char *title, const char *text );


/**
 * @brief Update the preloading screen's text fields.
 * @param[in,out] parent loading-bar screen
 * @param[in] title Statusbar title string. NULL to not update title.
 * @param[in] text subtitle text displayed above the spinning arc. NULL to not update subtitle.
 */
void jolt_gui_scr_preloading_update( jolt_gui_obj_t *parent, const char *title, const char *text );

#endif
