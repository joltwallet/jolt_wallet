/**
 * @file home.h
 * @brief Jolt GUI Home screen
 * @author Brian Pugh
 */

#ifndef __JOLT_GUI_MENU_HOME_H__
#define __JOLT_GUI_MENU_HOME_H__

/**
 * @brief Creates the Jolt GUI screen
 */
void jolt_gui_menu_home_create();

/**
 * @brief Deletes and recreates the home menu
 *
 * Typically used to refresh the application list.
 */
void jolt_gui_menu_home_refresh();

#endif
