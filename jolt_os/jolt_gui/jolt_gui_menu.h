#ifndef JOLT_GUI_MENU_H__
#define JOLT_GUI_MENU_H__

#include "lvgl/lvgl.h"


/* Creates a submenu of correct size, location, group, and infocus. 
 * Need to provide arguments for first element so focus works correctly.*/
lv_obj_t *jolt_gui_scr_menu_create(const char *title);

/* Adds an item to a Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_add(lv_obj_t *par, const void *img_src,
        const char *txt, lv_action_t rel_action);

/* Gets the list object of a menu screen */
lv_obj_t *jolt_gui_scr_menu_get_list(lv_obj_t *par);

#endif
