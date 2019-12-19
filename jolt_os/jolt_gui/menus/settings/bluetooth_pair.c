#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "hal/radio/bluetooth.h"
#include "jolt_gui/jolt_gui.h"

#if CONFIG_BT_ENABLED
static const char TAG[]             = "bluetooth_pair";
static jolt_gui_obj_t *passkey_scr  = NULL;
static jolt_gui_obj_t *scanning_scr = NULL;

static void scanning_scr_cb( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.cancel == event ) {
        jolt_bluetooth_pair_mode = false;
        jolt_bluetooth_adv_wht_start();
        jolt_gui_scr_del( btn );
        scanning_scr = NULL;
    }
}

static void passkey_scr_cb( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.cancel == event ) {
        jolt_bluetooth_pair_mode = false;
        jolt_bluetooth_adv_stop();
        jolt_gui_obj_del( passkey_scr );
        passkey_scr = NULL;
    }
}

static void menu_bluetooth_pair_common_vault_success_cb( void *dummy )
{
    jolt_bluetooth_pair_mode = true;
    jolt_bluetooth_adv_all_start();
    scanning_scr = jolt_gui_scr_preloading_create( gettext( JOLT_TEXT_PAIR ), gettext( JOLT_TEXT_BROADCASTING ) );
    jolt_gui_scr_set_event_cb( scanning_scr, scanning_scr_cb );
}

static void menu_bluetooth_pair_common_create()
{
    jolt_settings_vault_set( NULL, menu_bluetooth_pair_common_vault_success_cb, NULL );
}

void menu_bluetooth_pair_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_bluetooth_config_security( true ); /* enable bonding */
        menu_bluetooth_pair_common_create();
    }
}

void menu_bluetooth_temp_pair_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_bluetooth_config_security( false ); /* disable bonding */
        menu_bluetooth_pair_common_create();
    }
}

int jolt_gui_bluetooth_pair_gap_cb( struct ble_gap_event *event, void *arg )
{
    int rc;
    struct ble_gap_conn_desc desc;

    switch( event->type ) {
        case BLE_GAP_EVENT_PASSKEY_ACTION:
            ESP_LOGI( TAG, "PASSKEY_ACTION_EVENT GUI started \n" );
            struct ble_sm_io pkey = {0};

            assert( event->passkey.params.action == BLE_SM_IOACT_DISP );

            pkey.action  = event->passkey.params.action;
            pkey.passkey = jolt_random_range( 0, 1000000 );  // 6 digit number
            ESP_LOGD( TAG, "Enter passkey %06d on the peer side", pkey.passkey );
            rc = ble_sm_inject_io( event->passkey.conn_handle, &pkey );
            assert( 0 == rc );

            if( NULL != passkey_scr ) {
                JOLT_GUI_CTX
                {
                    jolt_gui_obj_del( passkey_scr );
                    passkey_scr = NULL;
                }
            }
            if( NULL != scanning_scr ) {
                JOLT_GUI_CTX
                {
                    jolt_gui_obj_del( scanning_scr );
                    scanning_scr = NULL;
                }
            }

            passkey_scr = jolt_gui_scr_bignum_create( "Bluetooth Pair", "Pairing Key", pkey.passkey, 6 );
            jolt_gui_scr_set_event_cb( passkey_scr, passkey_scr_cb );

            break;

        case BLE_GAP_EVENT_ENC_CHANGE:
            rc = ble_gap_conn_find( event->connect.conn_handle, &desc );
            assert( rc == 0 );

            if( desc.sec_state.encrypted && desc.sec_state.authenticated ) {
                if( NULL != passkey_scr ) {
                    JOLT_GUI_CTX
                    {
                        jolt_gui_obj_del( passkey_scr );
                        passkey_scr = NULL;
                    }
                }
                if( NULL != scanning_scr ) {
                    JOLT_GUI_CTX
                    {
                        jolt_gui_obj_del( scanning_scr );
                        scanning_scr = NULL;
                    }
                }
            }
            break;

        default: break;
    }

    return 0;
}

#endif
