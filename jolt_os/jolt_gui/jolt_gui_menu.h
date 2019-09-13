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
 * @brief Creates a scrollable menu that options can be added to.
 *
 * Default event handler: jolt_gui_event_del
 *
 * @param[in] title title-bar string
 * @return menu screen
 */
lv_obj_t *jolt_gui_scr_menu_create(const char *title);

/**
 * @brief Adds an item to a Jolt Menu Screen 
 * @param[in,out] par menu screen 
 * @param[in] img_src icon to display to the left of text; NULL for no image
 * @param[in] txt string to display
 * @param[in] event_cb Callback handler
 * @return the btn object added to the menu
 */
lv_obj_t *jolt_gui_scr_menu_add(lv_obj_t *par, const void *img_src,
        const char *txt, lv_event_cb_t event_cb);

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
 * @return lv_sw object.
 */
lv_obj_t *jolt_gui_scr_menu_add_sw( lv_obj_t *btn );

/**
 * @brief Displays a right-justified string for an element. This must be a short
 * string, or else it might totally eclipse the menu element's label.
 *
 * @return created lv_label object.
 */
lv_obj_t *jolt_gui_scr_menu_add_info( lv_obj_t *btn, const char *info );

/**
 * @brief Sets the current selection to btn 
 * @param[in,out] par Jolt menu screen
 * @param[in] btn menu button to select
 */
void jolt_gui_scr_menu_set_btn_selected(lv_obj_t *par, lv_obj_t *btn);

/**
 * @brief Deletes elements from indicies start to end (inclusive) 
 * @param[in,out] par Jolt menu screen
 * @param[in] start Starting index to delete (inclusive).
 * @param[in] end Ending index to delete (inclusive). Set to 0 to delete all past start.
 */
void jolt_gui_scr_menu_remove(lv_obj_t *par, uint16_t start, uint16_t end);

/**
 * @brief Set the active object param and all of the buttons
 * @param[in] par Menu screen
 * @param[in] param param to set.
 */
void jolt_gui_scr_menu_set_param( lv_obj_t *par, void *param );

/**
 * @brief Get the index of a button
 *
 * Useful for getting button position within a callback.
 *
 * @param btn Button that belongs to a menu.
 * @return 0-Index of button. Returns -1 on error.
 */
int32_t jolt_gui_scr_menu_get_btn_index( lv_obj_t *btn );

#endif
