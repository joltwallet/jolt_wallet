#ifndef JOLT_GUI_MENU_H__
#define JOLT_GUI_MENU_H__

#include "lvgl/lvgl.h"

#define JOLT_GUI_SW_WIDTH 20
#define JOLT_GUI_SW_HEIGHT 10
#define JOLT_GUI_SW_ANIM_TIME_MS 150
#define JOLT_GUI_SW_LEFT_PADDING 4
#define JOLT_GUI_SW_RIGHT_PADDING 1
#define JOLT_GUI_SW_CONT_WIDTH (JOLT_GUI_SW_LEFT_PADDING + JOLT_GUI_SW_RIGHT_PADDING + JOLT_GUI_SW_WIDTH)

/* Creates a submenu of correct size, location, group, and infocus. 
 * Need to provide arguments for first element so focus works correctly.*/
lv_obj_t *jolt_gui_scr_menu_create(const char *title);

/* Adds an item to a Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_add(lv_obj_t *par, const void *img_src,
        const char *txt, lv_action_t rel_action);

/* Gets the list object of a menu screen */
lv_obj_t *jolt_gui_scr_menu_get_list(lv_obj_t *par);

/* Adds an item to a Jolt Menu Screen */
lv_obj_t *jolt_gui_scr_menu_add_sw( lv_obj_t *btn );

/* Sets the current selection to btn */
void jolt_gui_scr_menu_set_btn_selected(lv_obj_t *par, lv_obj_t *btn);

/* Deletes elements from indicies start to end (inclusive) */
void jolt_gui_scr_menu_remove(lv_obj_t *par, uint16_t start, uint16_t end);

#endif
