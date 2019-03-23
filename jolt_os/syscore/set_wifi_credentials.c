#include "sodium.h"
#include "esp_log.h"
#include "esp_system.h"
#include "vault.h"
#include "hal/storage/storage.h"
#include "hal/radio/wifi.h"
#include "syscore/set_wifi_credentials.h"
#include "jolt_helpers.h"
#include "jolt_gui/jolt_gui.h"
#include "console.h"

#define JOLT_WIFI_SSID_LEN_MAX 32
#define JOLT_WIFI_PASS_LEN_MAX 64
#define JOLT_TEXT_WIFI_CRED_UPDATE_PROMPT_1 \
    "Update WiFi to:\nSSID: %s\nPassword: %s"

static const char TAG[] = "set_wifi_cred";
static const char title[] = "WiFi Update";

static char *target_ssid = NULL;
static char *target_pass = NULL;

static bool in_progress = false;

static void clear_vars() {
    if( NULL != target_ssid ){
        free(target_ssid);
        target_ssid = NULL;
    }
    if( NULL != target_pass ){
        sodium_memzero(target_pass, JOLT_WIFI_PASS_LEN_MAX);
        free(target_pass);
        target_pass = NULL;
    }
    in_progress = false;
}

static void vault_fail_cb(void *dummy) {
    ESP_LOGI(TAG, "vault_fail_cb");
    clear_vars();
    jolt_cmd_return(-1);
}

static void vault_success_cb(void *dummy) {
    ESP_LOGI(TAG, "vault_success_cb");
    storage_set_str(target_ssid, "user", "wifi_ssid");
    storage_set_str(target_pass, "user", "wifi_pass");
    clear_vars();
    // restart wifi
    uint8_t wifi_en;
    storage_get_u8(&wifi_en, "user", "wifi_en", 0 );
    if( wifi_en ) {
        jolt_wifi_start();
    }
    jolt_cmd_return(0);
}

static lv_res_t prompt_1_back_cb(lv_obj_t *btn){
    clear_vars();
    jolt_gui_scr_del( );
    jolt_cmd_return(-1);
    return LV_RES_INV;
}

static lv_res_t prompt_1_enter_cb(lv_obj_t *btn){
    jolt_gui_scr_del( );
    jolt_h_settings_vault_set(vault_fail_cb, vault_success_cb, NULL);
    return LV_RES_INV;
}

/* Non-blocking */
void set_wifi_credentials(const char *ssid, const char *pass) {
    char buf[strlen(JOLT_TEXT_WIFI_CRED_UPDATE_PROMPT_1) + JOLT_WIFI_SSID_LEN_MAX + JOLT_WIFI_PASS_LEN_MAX + 1];

    if( in_progress ) {
        printf("Wifi credential update already in progress.\n");
        return;
    }
    in_progress = true;

    /* Validate Inputs */
    if( NULL == ssid ){
        goto exit_error;
    }
    else if( strlen(ssid) > JOLT_WIFI_SSID_LEN_MAX ) {
        goto exit_error;
    }
    if( NULL != pass && strlen(pass) > JOLT_WIFI_PASS_LEN_MAX ){
        goto exit_error;
    }

    /* allocate and copy the user's SSID */
    target_ssid = malloc(JOLT_WIFI_SSID_LEN_MAX+1);
    if( NULL == target_ssid ) {
        goto exit_error;
    }
    strcpy(target_ssid, ssid);

    /* allocate and copy the user's password */
    target_pass = malloc(JOLT_WIFI_PASS_LEN_MAX+1);
    if( NULL == target_pass ) {
        goto exit_error;
    }
    if( NULL == pass ){
        *target_pass = '\0';
    }
    else{
        strcpy(target_pass, pass);
    }

    /* Create prompt screen */
    snprintf(buf, sizeof(buf), JOLT_TEXT_WIFI_CRED_UPDATE_PROMPT_1,
            target_ssid, target_pass);
    lv_obj_t *scr = jolt_gui_scr_text_create( title, buf );
    jolt_gui_scr_set_back_action( scr, prompt_1_back_cb );
    jolt_gui_scr_set_enter_action( scr, prompt_1_enter_cb );

    return;

exit_error:
    clear_vars();
}


