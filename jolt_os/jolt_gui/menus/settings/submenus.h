/**
 * @file submenus.h
 * @brief System settings submenu options
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_MENUS_SETTINGS_SUBMENUS_H__
#define JOLT_GUI_MENUS_SETTINGS_SUBMENUS_H__

#include "jolt_gui/jolt_gui.h"

/**
 * @brief Create general WiFi submenu screen
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_wifi_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Displays WiFi Details (Strength, IP Address, etc)
 *
 * Child element of WiFi submenu
 *
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_wifi_details_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Creates slider screen that controls display brightness.
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_screen_brightness_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Creates slider screen that controls touch LED brightness
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_led_brightness_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Creates the language select menu screen
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_language_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Screens to perform a Factory Reset
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_factory_reset_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Create general Bluetooth submenu screen
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_bluetooth_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Displays passcode to pair devices. Puts Jolt into pairing mode that bonds with device.
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_bluetooth_pair_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Displays passcode to pair devices. Does NOT bond with device.
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_bluetooth_temp_pair_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Screen to forget bluetooth devices
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_bluetooth_unbond_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief Storage screen displaying storage use.
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
void menu_storage_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

/**
 * @brief List of installed applications and their sizes.
 */
void menu_storage_details_create( jolt_gui_obj_t *btn, jolt_gui_event_t event );

#endif
