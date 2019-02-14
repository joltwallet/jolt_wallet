#include "jolt_gui/jolt_gui.h"
#include "hal/radio/bluetooth.h"
#include "hal/radio/bluetooth_state.h"

#if CONFIG_BT_ENABLED
static const char TAG[] = "bluetooth_pair";
static lv_obj_t *passkey_scr  = NULL;
static lv_obj_t *scanning_scr = NULL;

static lv_res_t scanning_scr_back_cb(lv_obj_t *btn) {
    jolt_bluetooth_pair_mode = false;
    jolt_bluetooth_adv_wht_start();
    jolt_gui_scr_del();
    scanning_scr = NULL;
    return LV_RES_INV;
}

static lv_res_t passkey_scr_back_cb(lv_obj_t *btn) {
    /* todo: This is not the correct action to take */
    jolt_bluetooth_pair_mode = false;
    jolt_bluetooth_adv_stop();
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_res_t menu_bluetooth_pair_common_create( lv_obj_t *btn ) {
    jolt_bluetooth_pair_mode = true;
    jolt_bluetooth_adv_all_start();
    scanning_scr = jolt_gui_scr_preloading_create(gettext(JOLT_TEXT_PAIR), gettext(JOLT_TEXT_BROADCASTING));
    jolt_gui_scr_set_back_action(scanning_scr, scanning_scr_back_cb);
    return LV_RES_OK;
}

lv_res_t menu_bluetooth_pair_create( lv_obj_t *btn ) {
    jolt_bluetooth_config_security( true ); /* enable bonding */
    return menu_bluetooth_pair_common_create( btn );
}

lv_res_t menu_bluetooth_temp_pair_create(lv_obj_t *btn) {
    jolt_bluetooth_config_security( false ); /* disable bonding */
    return menu_bluetooth_pair_common_create( btn );
}

void jolt_gui_bluetooth_pair_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch( event ) {
        case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
            // todo: internationalization
            if( NULL != passkey_scr ) {
                JOLT_GUI_CTX{
                    lv_obj_del( passkey_scr );
                    passkey_scr = NULL;
                }
            }
            if( NULL != scanning_scr ) {
                JOLT_GUI_CTX{
                    lv_obj_del( scanning_scr );
                    scanning_scr = NULL;
                }
            }

            passkey_scr = jolt_gui_scr_bignum_create("Bluetooth Pair",
                    "Pairing Key", param->ble_security.key_notif.passkey, 6);
            jolt_gui_scr_set_back_action(passkey_scr, passkey_scr_back_cb);
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            jolt_bluetooth_pair_mode = false;
            if( NULL != passkey_scr){
                JOLT_GUI_CTX{
                    lv_obj_del( passkey_scr );
                    passkey_scr = NULL;
                }
            }
            if( NULL != scanning_scr ) {
                JOLT_GUI_CTX{
                    lv_obj_del( scanning_scr );
                    scanning_scr = NULL;
                }
            }
            break;
        default:
            break;
    }
}
#endif
