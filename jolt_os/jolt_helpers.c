/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"
#include "esp_err.h"
#include "sdkconfig.h"

#include "esp_vfs_dev.h"
#include "esp_spiffs.h"

#include "jolttypes.h"
#include "bipmnemonic.h"

#include "vault.h"
#include "jolt_helpers.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/home.h"
#include "syscore/https.h"
#include "syscore/filesystem.h"
#include "esp_hdiffz.h"


static const char* TAG = "jolt_helpers";
const char NULL_TERM = '\0';
const char *EMPTY_STR = "";

void jolt_get_random(uint8_t *buf, uint8_t n_bytes){

#if CONFIG_JOLT_STORE_ATAES132A
    {
        ESP_LOGD(TAG, "Getting %d bytes from ATAES132A RNG Source", n_bytes);
        uint8_t res = aes132_rand(buf, n_bytes);
        if( ESP_OK != res ) {
            esp_restart();
        }
    }
#endif

    /* ESP32 Strong RNG source */
    {
        ESP_LOGD(TAG, "Getting %d bytes from ESP32 RNG Source", n_bytes);
        CONFIDENTIAL uint8_t rand_buffer[4];
        for(uint8_t i=0, j=0; i<n_bytes; i+=4){
            *(uint32_t *)rand_buffer = randombytes_random();
            for(uint8_t k=0; k < 4; k++){
                buf[j] ^= rand_buffer[k];
                j++;
                if(j==n_bytes) break;
            }
        }
        sodium_memzero(rand_buffer, sizeof(rand_buffer));
    }
}

void shuffle_arr(uint8_t *arr, int arr_len) {
    uint8_t tmp;
    for(int i=arr_len-1; i>0; i--) {
        /* todo: use unified random bytes here */
        uint32_t idx = randombytes_random() % (i+1);
        tmp = arr[idx];
        arr[idx] = arr[i];
        arr[i] = tmp;
    }
    sodium_memzero(&tmp, sizeof(uint8_t));
}

char **jolt_h_malloc_char_array(int n) {
    return (char **) calloc(n, sizeof(char*));
}

void jolt_h_free_char_array(char **arr, int n) {
    for(uint32_t i=0; i<n; i++) {
        free(arr[i]);
    }
    free(arr);
}

bool jolt_h_strcmp_suffix( const char *str, const char *suffix){
    uint32_t str_len = strlen(str);
    uint32_t suffix_len = strlen(suffix);

    if(suffix_len > str_len){
        return false;
    }

    if( 0 == strcmp( str+str_len-suffix_len, suffix ) ) {
        return true;
    }

    return false;
}

void jolt_h_fn_home_refresh(const char *str) {
    if( !jolt_h_strcmp_suffix(str, ".jelf") ) {
        return;
    }
    jolt_gui_menu_home_refresh();
}

void jolt_h_settings_vault_set(vault_cb_t fail_cb, vault_cb_t success_cb, void *param) {
    vault_set( JOLT_OS_DERIVATION_PURPOSE,
            JOLT_OS_DERIVATION_PATH,
            JOLT_OS_DERIVATION_BIP32_KEY,
            JOLT_OS_DERIVATION_PASSPHRASE,
            fail_cb, success_cb, param);
}

void jolt_h_apply_patch(const char *filename){
    // TODO: prompt user
    FILE *f_diff = NULL, *f_jelf = NULL, *f_tmp = NULL;
    char *path_diff = NULL;
    if( !jolt_h_strcmp_suffix(filename, ".patch") ) goto exit;

    /* Ensure its a full path */
    path_diff = jolt_fs_parse(filename, NULL);
    if(NULL == path_diff) goto exit;

    /* Open DIFF file */
    f_diff = fopen(path_diff, "rb");
    if(NULL == f_diff) goto exit;

    if( 0 == jolt_h_strcmp_suffix(path_diff, "joltos.patch") ) {
        /* Firmware Update */
        // TODO: GUI stuff
        esp_hdiffz_ota_file(f_diff);
        SAFE_CLOSE(f_diff);
        remove(path_diff);
        esp_restart();
    }
    else {
        /* File Update */
        esp_err_t err;

        jolt_fs_strip_ext(path_diff);
        strcat(path_diff, ".jelf"); // NOTE: only works because "jelf" is shorter than "patch"

        f_jelf = fopen(path_diff, "rb");
        if(f_jelf == NULL) goto exit;

        // Will output to /store/tmp
        f_tmp = fopen(JOLT_FS_TMP_FN, "wb");
        if(f_tmp == NULL) goto exit;

        err = esp_hdiffz_patch_file(f_jelf, f_tmp, f_diff);
        if( ESP_OK != err ) goto exit;

        SAFE_CLOSE(f_jelf);
        SAFE_CLOSE(f_tmp);
        SAFE_CLOSE(f_diff);

        /* Replace the app with the patched version */
        err = jolt_fs_mv(JOLT_FS_TMP_FN, path_diff);
        if(ESP_OK != err) goto exit;
    }

exit:
    SAFE_FREE(path_diff);
    SAFE_CLOSE(f_diff);
    SAFE_CLOSE(f_jelf);
    SAFE_CLOSE(f_tmp);
    return;
}

static SemaphoreHandle_t suspend_logging_sem = NULL;
void jolt_suspend_logging() {
    esp_log_level_set("*", ESP_LOG_NONE);

    if(NULL == suspend_logging_sem) {
        suspend_logging_sem = xSemaphoreCreateCounting( 200, 0); // arbitrarily high value.
        if(NULL == suspend_logging_sem) {
            esp_restart();
        }
    }

    xSemaphoreGive(suspend_logging_sem);

}

void jolt_resume_logging() {
    if( NULL == suspend_logging_sem || !xSemaphoreTake(suspend_logging_sem, 0) ) {
        assert(0);
        esp_restart();
    }
    if( 0 == uxSemaphoreGetCount(suspend_logging_sem) ) {
        esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);
        esp_log_level_set("wifi", ESP_LOG_NONE);
    }
}

