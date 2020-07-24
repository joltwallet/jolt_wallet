//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "sodium.h"
#include "syscore/bg.h"
#include "syscore/cli_helpers.h"
#include "syscore/launcher.h"

static const char TAG[] = "cmd_app_key";
static uint256_t app_key;

static int factory_reset_job( jolt_bg_job_t *job )
{
    ESP_LOGD( TAG, "Performing factory reset" );
    storage_factory_reset( false );

    ESP_LOGD( TAG, "Setting app_key" );
    if( !storage_set_blob( app_key, sizeof( app_key ), "user", "app_key" ) ) { printf( "Error setting app_key.\n" ); }
    printf( "Successfully set App Key.\n" );
    esp_restart();
}

static void set_app_key_cb( lv_obj_t *btn, lv_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        ESP_LOGD( TAG, "Creating preloading screen" );
        jolt_gui_scr_del( btn );
        jolt_gui_scr_preloading_create( "App Key", "Erasing..." );

        esp_err_t err;
        ESP_LOGD( TAG, "Dispatching factory reset job" );
        err = jolt_bg_create( factory_reset_job, NULL, NULL );
        if( ESP_OK != err ) { esp_restart(); }
    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( btn );
    }
}

int jolt_cmd_app_key( int argc, char **argv )
{
    int return_code = -1;

    /* Input Validation */
    if( !console_check_range_argc( argc, 1, 2 ) ) {
        return_code = 1;
        goto exit;
    }

    /* Print App Key */
    if( 1 == argc ) {
        uint256_t approved_pub_key;
        hex256_t pub_key_hex;
        size_t required_size;
        if( !storage_get_blob( NULL, &required_size, "user", "app_key" ) ) {
            printf( "Stored App Key not found\n" );
            return_code = 4;
            goto exit;
        }
        if( sizeof( approved_pub_key ) != required_size ||
            !storage_get_blob( approved_pub_key, &required_size, "user", "app_key" ) ) {
            printf( "Stored App Key Blob doesn't have expected len.\n" );
            return_code = 5;
            goto exit;
        }
        sodium_bin2hex( pub_key_hex, sizeof( pub_key_hex ), approved_pub_key, sizeof( approved_pub_key ) );
        printf( "App Key: %s\n", pub_key_hex );
        goto exit;
    }

    /* Set App Key */
    if( strlen( argv[1] ) != 64 ) {
        printf( "App Key must be 64 characters long in hexadecimal\n" );
        return_code = 2;
        goto exit;
    }
    ESP_ERROR_CHECK( sodium_hex2bin( app_key, sizeof( app_key ), argv[1], 64, NULL, NULL, NULL ) );

    /* Make sure we are not in an app */
    if( jolt_launch_in_app() ) {
        printf( "Cannot set app key while an app is running.\n" );
        return_code = 3;
        goto exit;
    }

    /* Display Text; Pressing any button returns to previous screen */
    lv_obj_t *scr;

    /* Prompt user */
    scr = jolt_gui_scr_text_create( gettext( JOLT_TEXT_APP_KEY_SET_CONFIRMATION_TITLE ),
                                    gettext( JOLT_TEXT_APP_KEY_SET_CONFIRMATION ) );
    if( NULL == scr ) goto exit;
    jolt_gui_scr_scroll_add_monospace_text( scr, argv[1] );
    jolt_gui_scr_set_event_cb( scr, set_app_key_cb );

    return 0;

exit:
    return return_code;
}
