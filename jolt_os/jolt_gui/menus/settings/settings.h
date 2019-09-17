/**
 * @file settings.h
 * @brief System settings GUI
 * @author Brian Pugh
 */

#ifndef __JOLT_GUI_MENU_SETTINGS_H__
#define __JOLT_GUI_MENU_SETTINGS_H__

/**
 * @brief Create the system settings menu screen
 * @param btn The lv_btn of the parenting menu that triggered the settings menu creation.
 * @return jolt menu screen
 */
void menu_settings_create( lv_obj_t *btn, lv_event_t event );

#endif
