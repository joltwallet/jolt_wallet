/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/settings/submenus.h"
#include "sdkconfig.h"

void menu_settings_create( lv_obj_t *btn, lv_event_t event )
{
    if( LV_EVENT_SHORT_CLICKED == event ) {
        lv_obj_t *scr = jolt_gui_scr_menu_create( gettext( JOLT_TEXT_SETTINGS ) );
        jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_SCREEN_BRIGHTNESS ), menu_screen_brightness_create );
        jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_LED_BRIGHTNESS ), menu_led_brightness_create );
#if !CONFIG_NO_BLOBS
        jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_WIFI ), menu_wifi_create );
#endif

#if CONFIG_BT_ENABLED
        jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_BLUETOOTH ), menu_bluetooth_create );
#endif

        jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_LANGUAGE ), menu_language_create );
        jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_STORAGE ), menu_storage_create );
        jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_FACTORY_RESET ), menu_factory_reset_create );
    }
}
