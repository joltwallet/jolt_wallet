/**
 * @file jolt_gui_loading.h
 * @brief Loading Screens
 * @author Brian Pugh
 */

#ifndef __JOLT_GUI_LOADING_H__
#define __JOLT_GUI_LOADING_H__

#include "../../lvgl/lvgl.h"

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
lv_obj_t *jolt_gui_scr_loadingbar_create(const char *title);

/**
 * @brief Update the loading-bar progress
 * @param[in,out] parent loading-bar screen
 * @param[in] title Statusbar title string. NULL for an empty title
 * @param[in] text subtitle text displayed above the loading-bar. NULL for an empty subtitle.
 * @param[in] percentage Current progress [0~100]
 */
void jolt_gui_scr_loadingbar_update(lv_obj_t *parent,
        const char *title, const char *text,
        uint8_t percentage);
/**
 * @brief Automatically updates the loading bar according to the value of progress [0~100]
 * @param[in,out] parent loading-bar screen
 * @param[in] progress Pointer to current progress [0~100] which will be periodically read from.
 */
void jolt_gui_scr_loadingbar_autoupdate(lv_obj_t *parent, int8_t *progress);

/**
 * @brief Cleanup the autoupdate task.
 *
 * This gets called internally when the parent is deleted. Usually end-user should never call this.
 *
 * @param[in,out] parent loading-bar screen
 */
void jolt_gui_scr_loadingbar_autoupdate_del(lv_obj_t *parent);

/**
 * @brief Creates a spinning arc preloading screen.
 *
 * Typically used for tasks that have unknown progress to show the user.
 *
 * @param[in] title Statusbar title string. NULL for an empty title
 * @param[in] text subtitle text displayed above the spinning arc. NULL for an empty subtitle.

 * @return preloading screen
 */
lv_obj_t *jolt_gui_scr_preloading_create(const char *title, const char *text);

/**
 * @brief Update the preloading screen's text fields.
 * @param[in,out] parent loading-bar screen
 * @param[in] title Statusbar title string. NULL to not update title.
 * @param[in] text subtitle text displayed above the spinning arc. NULL to not update subtitle.
 */
void jolt_gui_scr_preloading_update(lv_obj_t *parent,
        const char *title, const char *text);

#endif
