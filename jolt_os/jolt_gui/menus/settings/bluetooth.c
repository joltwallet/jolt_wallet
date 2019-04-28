#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/settings/submenus.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "hal/radio/bluetooth.h"
#include "hal/storage/storage.h"

static jolt_gui_obj_t *scr = NULL;
static jolt_gui_obj_t *sw_en = NULL;

static void create_enable_list();
static void create_disable_list();

static void create_enable_list() {
    /* Elements to show while bluetooth is enabled */
    jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_PAIR), menu_bluetooth_pair_create);
    jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_TEMP_PAIR), menu_bluetooth_temp_pair_create);
    create_disable_list();
}

static void create_disable_list() {
    /* Elements to show while bluetooth is disabled */
    jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_UNBONDS), menu_bluetooth_unbond_create);
}

static void destroy_list() {
    /* Delete everything after the enable/disable element */
     jolt_gui_scr_menu_remove(scr, 1, 0);
}

static void sw_en_cb(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t state;
        state = lv_sw_toggle(sw_en, true);

        if( state ) {
            create_enable_list();
            if( ESP_OK != jolt_bluetooth_start() ) {
                lv_sw_toggle(sw_en, false);
                return;
            }
        }
        else {
            destroy_list();
            create_disable_list();
            if( ESP_OK != jolt_bluetooth_stop() ) {
                /* Bluetooth wasn't successfully stopped */
                lv_sw_toggle(sw_en, false);
                return;
            }
        }

        storage_set_u8(state, "user", "bluetooth_en");
    }
}

void menu_bluetooth_create(jolt_gui_obj_t *btn, jolt_gui_event_t event) {
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t bluetooth_en;
        storage_get_u8(&bluetooth_en, "user", "bluetooth_en", 0 );

        scr = jolt_gui_scr_menu_create(gettext(JOLT_TEXT_SETTINGS));
        jolt_gui_obj_t *btn_en = jolt_gui_scr_menu_add(scr, NULL, gettext(JOLT_TEXT_BLUETOOTH_ENABLE), sw_en_cb);
        sw_en = jolt_gui_scr_menu_add_sw( btn_en );

        if( bluetooth_en ) {
            lv_sw_on( sw_en, false );
            create_enable_list();
        }
        else{
            lv_sw_off( sw_en, false);
        }
    }
}

#endif
