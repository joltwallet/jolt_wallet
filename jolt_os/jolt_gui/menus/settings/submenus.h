/**
 * @file submenus.h
 * @brief System settings submenu options
 * @author Brian Pugh
 */

#ifndef JOLT_GUI_MENUS_SETTINGS_SUBMENUS_H__
#define JOLT_GUI_MENUS_SETTINGS_SUBMENUS_H__

#include "lvgl/lvgl.h"

/**
 * @brief Create general WiFi submenu screen
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_wifi_create(lv_obj_t *btn);

/**
 * @brief Displays WiFi Details (Strength, IP Address, etc)
 *
 * Child element of WiFi submenu
 *
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_wifi_details_create(lv_obj_t *btn);

/**
 * @brief Creates slider screen that controls display brightness.
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_screen_brightness_create();

/**
 * @brief Creates slider screen that controls touch LED brightness
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_led_brightness_create();

/**
 * @brief Creates the language select menu screen
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_language_create(lv_obj_t *btn);

/**
 * @brief Screens to perform a Factory Reset
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_factory_reset_create(lv_obj_t *btn);

/**
 * @brief Create general Bluetooth submenu screen
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_bluetooth_create(lv_obj_t *btn);

/**
 * @brief Displays passcode to pair devices. Puts Jolt into pairing mode that bonds with device.
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_bluetooth_pair_create(lv_obj_t *btn);

/**
 * @brief Displays passcode to pair devices. Does NOT bond with device.
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_bluetooth_temp_pair_create(lv_obj_t *btn);

/**
 * @brief Screen to forget bluetooth devices
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_bluetooth_unbond_create(lv_obj_t *btn);

/**
 * @brief Storage screen displaying storage use. 
 * @param[in] btn The lv_btn of the settings menu that triggered this function.
 * @return screen object
 */
lv_res_t menu_storage_create( lv_obj_t *btn );

#endif
