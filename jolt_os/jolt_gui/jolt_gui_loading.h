#ifndef __JOLT_GUI_LOADING_H__
#define __JOLT_GUI_LOADING_H__

#include "../../lvgl/lvgl.h"

#ifndef CONFIG_JOLT_GUI_LOADING_BAR_W
    #define CONFIG_JOLT_GUI_LOADING_BAR_W 100
#endif
#ifndef CONFIG_JOLT_GUI_LOADING_BAR_H
    #define CONFIG_JOLT_GUI_LOADING_BAR_H 15
#endif
#ifndef CONFIG_JOLT_GUI_LOADING_BAR_ANIM_MS
    #define CONFIG_JOLT_GUI_LOADING_BAR_ANIM_MS 300
#endif

lv_obj_t *jolt_gui_scr_loading_create(const char *title);

void jolt_gui_scr_loading_update(lv_obj_t *parent,
        const char *title, const char *text,
        uint8_t percentage);

/* Automatically updates the loading bar according to the value of 
 * 0-100 percentage of progress */
void jolt_gui_scr_loading_autoupdate(lv_obj_t *parent, int8_t *progress);

/* Cleanup the autoupdate task. This gets called internally when the parent is
 * deleted. Usually end-user should never call this */
void jolt_gui_scr_loading_autoupdate_del(lv_obj_t *parent);

/* Preloading is a spinning arc. Usually used for activities with unknown 
 * progress. */
lv_obj_t *jolt_gui_scr_preloading_create(const char *title, const char *text);

void jolt_gui_scr_preloading_update(lv_obj_t *parent,
        const char *title, const char *text);

#endif
