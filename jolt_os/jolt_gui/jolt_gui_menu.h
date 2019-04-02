/**
 * @file jolt_gui_menu.h
 * @brief Scrollable menu's making up the main navigation of JoltOS
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_MENU_H__
#define JOLT_GUI_MENU_H__

#include "lvgl/lvgl.h"

#define JOLT_GUI_SW_WIDTH 20
#define JOLT_GUI_SW_HEIGHT 10
#define JOLT_GUI_SW_ANIM_TIME_MS 150
#define JOLT_GUI_SW_LEFT_PADDING 6
#define JOLT_GUI_SW_RIGHT_PADDING 1
#define JOLT_GUI_SW_CONT_WIDTH (JOLT_GUI_SW_LEFT_PADDING + JOLT_GUI_SW_RIGHT_PADDING + JOLT_GUI_SW_WIDTH)

/**
 * @brief Creates a scrollable menu that options can be added to
 * @param[in] title title-bar string
 * @return menu screen
 */
lv_obj_t *jolt_gui_scr_menu_create(const char *title);

/**
 * @brief Adds an item to a Jolt Menu Screen 
 * @param[in,out] par menu screen 
 * @param[in] img_src icon to display to the left of text; NULL for no image
 * @param[in] txt string to display
 * @param[in] rel_action action to take when this option is clicked on
 * @return the btn object added to the menu
 */
lv_obj_t *jolt_gui_scr_menu_add(lv_obj_t *par, const void *img_src,
        const char *txt, lv_action_t rel_action);

/**
 * @brief Gets the list object of a menu screen 
 * @param[in] par menu screen
 * @return lv_list object
 */
lv_obj_t *jolt_gui_scr_menu_get_list(lv_obj_t *par);

/**
 * @brief Gets the screen object given a selected button of a menu.
 *
 * Useful for getting the screen in btn callbacks
 * @param[in] par menu screen
 * @return jolt menu screen object
 */ 
lv_obj_t *jolt_gui_scr_menu_get_scr( lv_obj_t *btn );

/**
 * @brief Adds an item to a Jolt Menu Screen 
 * @param[in,out] btn Menu element to add a switch to
 * @return lv_sw object
 */
lv_obj_t *jolt_gui_scr_menu_add_sw( lv_obj_t *btn );

/**
 * @brief Sets the current selection to btn 
 * @param[in,out] par Jolt menu screen
 * @param[in] btn menu button to select
 */
void jolt_gui_scr_menu_set_btn_selected(lv_obj_t *par, lv_obj_t *btn);

/**
 * @brief Deletes elements from indicies start to end (inclusive) 
 * @param[in,out] par Jolt menu screen
 * @param[in] start Starting index to delete (inclusive)
 * @param[in] end Ending index to delete (inclusive)
 */
void jolt_gui_scr_menu_remove(lv_obj_t *par, uint16_t start, uint16_t end);

#endif
