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
#include "nano_rest.h"

#include "esp_vfs_dev.h"
#include "esp_spiffs.h"

#include "jolttypes.h"
#include "bipmnemonic.h"

#include "jolt_helpers.h"
#include "hal/storage/storage.h"
#include "jolt_globals.h"


static const char* TAG = "helpers";

void set_jolt_cast() {
    /* Sets Jolt Cast Server Params */
    size_t required_size;
    char *buf = NULL;
    uint16_t port;

    if( !storage_get_str(NULL, &required_size, "user", "jc_domain", CONFIG_JOLT_CAST_DOMAIN) ) {
        goto reset;
    }
    buf = malloc(required_size);
    storage_get_str(buf, &required_size, "user", "jc_domain", CONFIG_JOLT_CAST_DOMAIN);
    nano_rest_set_remote_domain(buf);
    free(buf);

    if( !storage_get_str(NULL, &required_size, "user", "jc_path", CONFIG_JOLT_CAST_PATH) ) {
        goto reset;
    }
    buf = malloc(required_size);
    storage_get_str(buf, &required_size, "user", "jc_path", CONFIG_JOLT_CAST_PATH);
    nano_rest_set_remote_path(buf);
    free(buf);

    if( !storage_get_u16(&port, "user", "jc_port", CONFIG_JOLT_CAST_PORT) ) {
        goto reset;
    }
    nano_rest_set_remote_path(buf);
    return;

reset:
    nano_rest_set_remote_domain(CONFIG_JOLT_CAST_DOMAIN);
    nano_rest_set_remote_path(CONFIG_JOLT_CAST_PATH);
    nano_rest_set_remote_port(CONFIG_JOLT_CAST_PORT);
    return;
}

uint8_t get_display_brightness() {
    /* Returns saved brightness or default */
    uint8_t brightness;
    storage_get_u8(&brightness, "user", "disp_bright", CONFIG_JOLT_DISPLAY_BRIGHTNESS);
    ESP_LOGI(TAG,"brightness %d", brightness);
    return brightness;
}

void save_display_brightness(uint8_t brightness) {
    storage_set_u8(brightness, "user", "disp_bright");
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


