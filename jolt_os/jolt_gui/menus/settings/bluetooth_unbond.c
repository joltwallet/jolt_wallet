#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"

#if CONFIG_BT_ENABLED

    #include "esp_gap_ble_api.h"
    #include "hal/radio/bluetooth.h"

static const char TAG[] = "bt_unbond";

static void remove_all_bonded_devices()
{
    int dev_num = esp_ble_get_bond_device_num();
    ESP_LOGI( TAG, "Removing %d bonded ble devices.", dev_num );

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc( sizeof( esp_ble_bond_dev_t ) * dev_num );
    esp_ble_get_bond_device_list( &dev_num, dev_list );
    for( int i = 0; i < dev_num; i++ ) {
        /* Attempt to remove it from the whitelist */
        esp_ble_gap_update_whitelist( JOLT_BLE_WHITELIST_REMOVE, dev_list[i].bd_addr, JOLT_BLE_WHITELIST_ADDR_TYPE );
        esp_ble_remove_bond_device( dev_list[i].bd_addr );
    }

    free( dev_list );
}

void menu_bluetooth_unbond_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    /* todo: confirmation screens.
     * todo: internationalization */
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_scr_text_create( gettext( JOLT_TEXT_UNBONDS ), "All saved bluetooth devices wiped." );
        remove_all_bonded_devices();
    }
}

#endif
