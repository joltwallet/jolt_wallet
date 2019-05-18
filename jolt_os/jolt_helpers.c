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


static const char* TAG = "jolt_helpers";

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
