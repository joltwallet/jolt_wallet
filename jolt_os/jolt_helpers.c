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
#include "syscore/https.h"


static const char* TAG = "helpers";

/* Sets */
void set_jolt_cast() {
    size_t required_size;
    char *uri = NULL;

    /* Sets Jolt Cast Server Params from NVS*/
    storage_get_str(NULL, &required_size, "user", "jc_uri", CONFIG_JOLT_CAST_URI);
    uri = malloc(required_size);
    if( NULL == uri ) {
        ESP_ERROR_CHECK( jolt_network_client_init( CONFIG_JOLT_CAST_URI ) );
    }
    else {
        storage_get_str(uri, &required_size, "user", "jc_uri", CONFIG_JOLT_CAST_URI);
        ESP_ERROR_CHECK( jolt_network_client_init( uri ) );
        free(uri);
    }
    return;
}

void shuffle_arr(uint8_t *arr, int arr_len) {
    /* Fisher Yates random shuffling */
    uint8_t tmp;
    for(int i=arr_len-1; i>0; i--) {
        uint32_t idx = randombytes_random() % (i+1);
        tmp = arr[idx];
        arr[idx] = arr[i];
        arr[i] = tmp;
    }
    sodium_memzero(&tmp, sizeof(uint8_t));
}

char **jolt_h_malloc_char_array(int n) {
    /* Allocate the pointers for a string array */
    return (char **) calloc(n, sizeof(char*));
}

void jolt_h_free_char_array(char **arr, int n) {
    /* Frees the list created by get_all_fns(); */
    for(uint32_t i=0; i<n; i++) {
        free(arr[i]);
    }
    free(arr);
}

/* Check if the provided null-terminated string suffix matches */
bool jolt_h_strcmp_suffix( char *str, char *suffix){
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

void jolt_h_fn_home_refresh(char *str) {
    if( jolt_h_strcmp_suffix(str, ".jelf") ) {
        jolt_gui_menu_home_refresh();
    }
}


/* Set vault for JoltOS settings stuff like bluetooth and wifi */
void jolt_h_settings_vault_set(vault_cb_t fail_cb, vault_cb_t success_cb) {
    vault_set( JOLT_OS_DERIVATION_PURPOSE,
            JOLT_OS_DERIVATION_PATH, JOLT_OS_DERIVATION_BIP32_KEY, "",
            fail_cb, success_cb, NULL);
}
