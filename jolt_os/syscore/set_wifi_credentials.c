
#define LOG_LOCAL_LEVEL 4

#include "syscore/set_wifi_credentials.h"
#include "esp_log.h"
#include "esp_system.h"
#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "sodium.h"
#include "syscore/cli.h"
#include "vault.h"

#define JOLT_WIFI_SSID_LEN_MAX 32
#define JOLT_WIFI_PASS_LEN_MAX 64

static const char TAG[]   = "set_wifi_cred";
static const char title[] = "WiFi Update";

static char *target_ssid = NULL;
static char *target_pass = NULL;

static bool in_progress = false;

static void clear_vars()
{
    if( NULL != target_ssid ) {
        free( target_ssid );
        target_ssid = NULL;
    }
    if( NULL != target_pass ) {
        sodium_memzero( target_pass, JOLT_WIFI_PASS_LEN_MAX );
        free( target_pass );
        target_pass = NULL;
    }
    in_progress = false;
}

static void vault_fail_cb( void *dummy )
{
    ESP_LOGI( TAG, "vault_fail_cb" );
    clear_vars();
    jolt_cli_return( -1 );
}

static void vault_success_cb( void *dummy )
{
    esp_err_t err = ESP_OK;
    ESP_LOGI( TAG, "vault_success_cb" );
    storage_set_str( target_ssid, "user", "wifi_ssid" );
    storage_set_str( target_pass, "user", "wifi_pass" );
    clear_vars();
    if( jolt_wifi_get_en() ) {
        ESP_LOGD( TAG, "WiFi restart" );
        err = jolt_wifi_start();
    }
    else {
        ESP_LOGD( TAG, "WiFi not enabled" );
    }
    jolt_cli_return( err );
}

static void prompt_1_cb( lv_obj_t *btn, lv_event_t event )
{
    if( LV_EVENT_SHORT_CLICKED == event ) {
        jolt_gui_scr_del( btn );
        jolt_settings_vault_set( vault_fail_cb, vault_success_cb, NULL );
    }
    else if( LV_EVENT_CANCEL == event ) {
        clear_vars();
        jolt_gui_scr_del( btn );
        jolt_cli_return( -1 );
    }
}

/* Non-blocking */
void set_wifi_credentials( const char *ssid, const char *pass )
{
    char buf[100 + JOLT_WIFI_SSID_LEN_MAX + JOLT_WIFI_PASS_LEN_MAX + 1] = {0};

    if( in_progress ) {
        printf( "Wifi credential update already in progress.\n" );
        return;
    }
    in_progress = true;

    /* Validate Inputs */
    if( NULL == ssid ) { goto exit_error; }
    else if( strlen( ssid ) > JOLT_WIFI_SSID_LEN_MAX ) {
        goto exit_error;
    }
    if( NULL != pass && strlen( pass ) > JOLT_WIFI_PASS_LEN_MAX ) { goto exit_error; }

    /* allocate and copy the user's SSID */
    target_ssid = strdup( ssid );
    if( NULL == target_ssid ) goto exit_error;

    /* allocate and copy the user's password */
    if( ( NULL == pass ) && ( NULL == ( target_pass = calloc( 1, 1 ) ) ) )
        goto exit_error;
    else if( NULL == ( target_pass = strdup( pass ) ) )
        goto exit_error;

    /* Create prompt screen */
    snprintf( buf, sizeof( buf ), gettext( JOLT_TEXT_WIFI_UPDATE ), target_ssid, target_pass );
    lv_obj_t *scr = jolt_gui_scr_text_create( title, buf );
    jolt_gui_scr_set_event_cb( scr, prompt_1_cb );

    return;

exit_error:
    clear_vars();
}
