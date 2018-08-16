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
#include "nano_lws.h"

#include "esp_vfs_dev.h"
#include "esp_spiffs.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "jolttypes.h"
#include "bipmnemonic.h"

#include "gui/entry.h"
#include "gui/statusbar.h"
#include "helpers.h"
#include "hal/storage.h"
#include "globals.h"


static const char* TAG = "helpers";

uint32_t fs_free() {
    uint32_t tot, used;
    esp_spiffs_info(NULL, &tot, &used);
    return (tot-used-16384);
}

size_t get_file_size(char *fname) {
    if (!esp_spiffs_mounted( NULL )) {
        return -1;
    }

    struct stat sb;
    if (stat(fname, &sb) == 0) {
        return sb.st_size;
    }
    else{
        return -1;
    }
}

int check_file_exists(char *fname) {
    if (!esp_spiffs_mounted( NULL )) {
        return -1;
    }

    struct stat sb;
    if (stat(fname, &sb) == 0) {
        return 1;
    }
    return 0;
}

void set_jolt_cast() {
    /* Sets Jolt Cast Server Params */
    size_t required_size;
    char *buf = NULL;
    uint16_t port;

    if( !storage_get_str(NULL, &required_size, "user", "jc_domain", NULL) ) {
        goto reset;
    }
    buf = malloc(required_size);
    storage_get_str(buf, &required_size, "user", "jc_domain", NULL);
    nano_lws_set_remote_domain(buf);
    free(buf);

    if( !storage_get_str(NULL, &required_size, "user", "jc_path", NULL) ) {
        goto reset;
    }
    buf = malloc(required_size);
    storage_get_str(buf, &required_size, "user", "jc_path", NULL);
    nano_lws_set_remote_path(buf);
    free(buf);

    if( !storage_get_u16(&port, "user", "jc_port", 0) ) {
        goto reset;
    }
    nano_lws_set_remote_path(buf);
    return;

reset:
    nano_lws_set_remote_domain(NULL);
    nano_lws_set_remote_path(NULL);
    nano_lws_set_remote_port(0);
    return;
}

uint8_t get_display_brightness() {
    /* Returns saved brightness or default */
    uint8_t brightness;
    storage_get_u8(&brightness, "user", "disp_bright", CONFIG_JOLT_DISPLAY_BRIGHTNESS);
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
