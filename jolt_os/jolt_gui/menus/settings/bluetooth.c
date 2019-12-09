#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

    #include "hal/radio/bluetooth.h"
    #include "hal/storage/storage.h"
    #include "jolt_gui/jolt_gui.h"
    #include "jolt_gui/menus/settings/submenus.h"

static jolt_gui_obj_t *scr   = NULL;
static jolt_gui_obj_t *sw_en = NULL;

static void create_enable_list();
static void create_disable_list();

/**
 * @brief Elements to show while bluetooth is enabled
 */
static void create_enable_list()
{
    jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_PAIR ), menu_bluetooth_pair_create );
    jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_TEMP_PAIR ), menu_bluetooth_temp_pair_create );
    create_disable_list();
}

/**
 * @brief Elements to show while bluetooth is disabled
 */
static void create_disable_list()
{
    jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_UNBONDS ), menu_bluetooth_unbond_create );
}

static void destroy_list()
{
    /* Delete everything after the enable/disable element */
    jolt_gui_scr_menu_remove_indices( scr, 1, 0 );
}

static void sw_en_cb( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t state;
        state = lv_sw_toggle( sw_en, true );

        if( state ) {
            destroy_list();
            create_enable_list();
            if( ESP_OK != jolt_bluetooth_start() ) {
                lv_sw_toggle( sw_en, false );
                return;
            }
        }
        else {
            destroy_list();
            create_disable_list();
            if( ESP_OK != jolt_bluetooth_stop() ) {
                /* Bluetooth wasn't successfully stopped */
                lv_sw_toggle( sw_en, false );
                return;
            }
        }

        storage_set_u8( state, "user", "bluetooth_en" );
    }
}

void menu_bluetooth_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        uint8_t bluetooth_en;
    #if CONFIG_JOLT_BT_ENABLE_DEFAULT
        storage_get_u8( &bluetooth_en, "user", "bluetooth_en", 1 );
    #else
        storage_get_u8( &bluetooth_en, "user", "bluetooth_en", 0 );
    #endif

        scr = jolt_gui_scr_menu_create( gettext( JOLT_TEXT_SETTINGS ) );
        if( NULL == scr ) return;
        jolt_gui_obj_t *btn_en = jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_BLUETOOTH_ENABLE ), sw_en_cb );
        sw_en                  = jolt_gui_scr_menu_add_sw( btn_en );

        if( bluetooth_en ) {
            lv_sw_on( sw_en, false );
            create_enable_list();
        }
        else {
            lv_sw_off( sw_en, false );
        }
    }
}

#endif
