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
#include "nvs_flash.h"
#include "nvs.h"
#include "helpers.h"


static const char* TAG = "helpers";

uint32_t fs_free() {
    uint32_t tot, used;
    esp_err_t ret = esp_spiffs_info(NULL, &tot, &used);
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

static void reset_jolt_cast_param() {
    nano_lws_set_remote_domain(NULL);
    nano_lws_set_remote_path(NULL);
    nano_lws_set_remote_port(0);
}

void set_jolt_cast() {
    /* Sets Jolt Cast Server Params */
    nvs_handle nvs_user;
    if(E_SUCCESS == init_nvm_namespace(&nvs_user, "user")){
        size_t required_size;
        char *tmp = NULL;
        if( ESP_OK != nvs_get_str(nvs_user, "jc_domain", NULL, &required_size) ) {
            reset_jolt_cast_param();
            return;
        }
        tmp = malloc(required_size);
        nvs_get_str(nvs_user, "jc_domain", tmp, &required_size);
        nvs_close(nvs_user);
        nano_lws_set_remote_domain(tmp);
        free(tmp);
    }
    else {
        reset_jolt_cast_param();
        return;
    }
    if(E_SUCCESS == init_nvm_namespace(&nvs_user, "user")){
        size_t required_size;
        char *tmp = NULL;
        if( ESP_OK != nvs_get_str(nvs_user, "jc_path", NULL, &required_size) ) {
            reset_jolt_cast_param();
            return;
        }
        tmp = malloc(required_size);
        nvs_get_str(nvs_user, "jc_path", tmp, &required_size);
        nvs_close(nvs_user);
        nano_lws_set_remote_path(tmp);
        free(tmp);
    }
    else {
        reset_jolt_cast_param();
        return;
    }
    if(E_SUCCESS == init_nvm_namespace(&nvs_user, "user")){
        uint16_t port;
        if( ESP_OK != nvs_get_u16(nvs_user, "jc_port", &port) ){
            reset_jolt_cast_param();
            return;
        }
        nvs_close(nvs_user);
        nano_lws_set_remote_port(port);
    }
    else {
        reset_jolt_cast_param();
        return;
    }
}

uint8_t get_display_brightness(){
    /* Returns saved brightness or default */
    uint8_t brightness = CONFIG_JOLT_DISPLAY_BRIGHTNESS;
    nvs_handle nvs_user;
    if(E_SUCCESS == init_nvm_namespace(&nvs_user, "user")){
        nvs_get_u8(nvs_user, "disp_bright", &brightness);
        nvs_close(nvs_user);
    }
    return brightness;
}

void save_display_brightness(uint8_t brightness){
    /* Returns saved brightness or default */
    nvs_handle nvs_user;
    if(E_SUCCESS == init_nvm_namespace(&nvs_user, "user")){
        nvs_set_u8(nvs_user, "disp_bright", brightness);
        nvs_close(nvs_user);
    }
}

jolt_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace){
    // Initialize NVS
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    
    // Open
    err = nvs_open(namespace, NVS_READWRITE, nvs_h);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%d) opening NVS handle with namespace %s!", err, namespace);
        return E_FAILURE;
    } else {
        ESP_LOGI(TAG, "Successfully opened NVM with namespace %s!", namespace);
        return E_SUCCESS;
    }
}

void factory_reset(){
    nvs_handle h;

    init_nvm_namespace(&h, "secret");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    init_nvm_namespace(&h, "user");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    init_nvm_namespace(&h, "nano");
    nvs_erase_all(h);
    nvs_commit(h);
    nvs_close(h);

    esp_restart();

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

void setup_menu8g2(menu8g2_t *menu) {
    // Usual menu8g2 initialization
    menu8g2_init(menu, (u8g2_t *) &u8g2, input_queue, disp_mutex, NULL, statusbar_update);
}

