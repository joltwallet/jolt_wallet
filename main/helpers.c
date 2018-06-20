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

#include "u8g2.h"
#include "menu8g2.h"
#include "gui/entry.h"
#include "nano_lib.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "helpers.h"


static const char* TAG = "helpers";

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

nl_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace){
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

void store_mnemonic_reboot(menu8g2_t *menu, char *mnemonic){
    /* Confirms pin, encrypts mnemonic into enc .
     * Returns if user cancels, restarts esp on success*/
    CONFIDENTIAL unsigned char enc_mnemonic[
            crypto_secretbox_MACBYTES + MNEMONIC_BUF_LEN];

    while(true){
        CONFIDENTIAL uint256_t pin_hash;
        if( !entry_pin(menu, pin_hash, "Set PIN") ){
            return false;
        }
        CONFIDENTIAL uint256_t pin_hash_verify;
        if( !entry_pin(menu, pin_hash_verify, "Verify PIN")){
            continue;
        }

        // Verify the pins match
        if( 0 == memcmp(pin_hash, pin_hash_verify, 32) ){
            uint256_t nonce = {0};
            sodium_memzero(pin_hash_verify, 32);
            // encrypt; only purpose is to reduce mnemonic redundancy to make a
            // frozen data remanence attack infeasible. Also convenient pin
            // checking. Nonce is irrelevant for this encryption
            crypto_secretbox_easy(enc_mnemonic, (unsigned char *) mnemonic, 
                    MNEMONIC_BUF_LEN, nonce, pin_hash);
            sodium_memzero(pin_hash, 32);
            break;
        }
        else{
            menu8g2_display_text_title(menu,
                    "Pin Mismatch! Please try again.",
                    "Pin Setup");
        }
    }

    // Save everything to NVS
    nvs_handle h;
    init_nvm_namespace(&h, "secret");
    nvs_erase_all(h);
    nvs_set_blob(h, "mnemonic", enc_mnemonic, sizeof(enc_mnemonic));
    nvs_set_u8(h, "pin_attempts", 0);
    nvs_commit(h);
    nvs_close(h);
    sodium_memzero(enc_mnemonic, sizeof(enc_mnemonic));

    esp_restart();
}


