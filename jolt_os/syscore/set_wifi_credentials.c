#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "sodium.h"
#include "esp_log.h"
#include "esp_system.h"
#include "vault.h"
#include "hal/storage/storage.h"
#include "syscore/set_wifi_credentials.h"
#include "jolt_helpers.h"
#include "jolt_gui/jolt_gui.h"

#define JOLT_WIFI_SSID_LEN_MAX 32
#define JOLT_WIFI_PASS_LEN_MAX 64
#define JOLT_TEXT_WIFI_CRED_UPDATE_PROMPT_1 \
    "Would you like to update your WiFi settings to:\nSSID: %s\nPassword: %s"

static const char TAG[] = "set_wifi_cred";
static const char title[] = "WiFi Update";

static char *target_ssid = NULL;
static char *target_pass = NULL;
static QueueHandle_t res_q = NULL;

static void report_fail(){
    bool res = false;
    xQueueSend(res_q, &res, portMAX_DELAY);
}
static void report_success(){
    bool res = true;
    xQueueSend(res_q, &res, portMAX_DELAY);
}

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
}

static lv_res_t vault_fail_cb(lv_obj_t *dummy) {
    clear_vars();
    jolt_gui_scr_del();
    report_fail();
    return LV_RES_INV;
}

static lv_res_t vault_success_cb(lv_obj_t *dummy) {
    storage_set_str(target_ssid, "user", "wifi_ssid");
    storage_set_str(target_pass, "user", "wifi_pass");
    report_success();
    return LV_RES_OK;
}

static lv_res_t prompt_1_back_cb(lv_obj_t *btn){
    clear_vars();
    jolt_gui_scr_del( );
    report_fail();
    return LV_RES_INV;
}

static lv_res_t prompt_1_enter_cb(lv_obj_t *btn){
    jolt_gui_scr_del( );
    ESP_LOGI(TAG, "Screen deleted, setting vault");
    jolt_h_settings_vault_set(vault_fail_cb, vault_success_cb);
    return LV_RES_INV;
}

/* Blocks until total process is complete */
bool set_wifi_credentials(char *ssid, char *pass) {
    char buf[strlen(JOLT_TEXT_WIFI_CRED_UPDATE_PROMPT_1) + JOLT_WIFI_SSID_LEN_MAX + JOLT_WIFI_PASS_LEN_MAX + 1];

    /* Validate Inputs */
    if( NULL == ssid ){
        goto exit_error;
    }
    else if( strlen(ssid) > JOLT_WIFI_SSID_LEN_MAX ) {
        goto exit_error;
    }
    else{
        target_ssid = malloc(JOLT_WIFI_SSID_LEN_MAX+1);
        if( NULL == target_ssid ) {
            goto exit_error;
        }
        strcpy(target_ssid, ssid);
    }

    if( NULL != pass ){
        if( strlen(pass) > JOLT_WIFI_PASS_LEN_MAX ){
            goto exit_error;
        }
        target_pass = malloc(JOLT_WIFI_PASS_LEN_MAX+1);
        if( NULL == target_pass ) {
            goto exit_error;
        }
        strcpy(target_pass, pass);
    }
    else{
        *target_pass = '\0';
    }

    /* Create queue for end result */
    res_q = xQueueCreate( 1, sizeof(bool) );
    if( NULL == res_q ) {
        goto exit_error;
    }

    /* Create prompt screen */
    snprintf(buf, sizeof(buf), JOLT_TEXT_WIFI_CRED_UPDATE_PROMPT_1,
            target_ssid, target_pass);
    lv_obj_t *scr = jolt_gui_scr_text_create( title, buf );
    jolt_gui_scr_set_back_action( scr, prompt_1_back_cb );
    jolt_gui_scr_set_enter_action( scr, prompt_1_enter_cb );

    /* Wait for user actions to complete */
    bool res;
    xQueueReceive(res_q, &res, portMAX_DELAY);
    vQueueDelete(res_q);
    res_q = NULL;

    return res;

exit_error:
    clear_vars();
    return false;
}


