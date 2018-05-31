/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "secure_entry.h"
#include "nano_lib.h"
#include "nvs_flash.h"
#include "nvs.h"


static const char* TAG = "helpers";

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

void store_mnemonic_reboot(menu8g2_t *menu, char *mnemonic){
    /* Confirms pin, encrypts mnemonic into enc .
     * Returns if user cancels, restarts esp on success*/
    CONFIDENTIAL unsigned char enc_mnemonic[
            crypto_secretbox_MACBYTES + MNEMONIC_BUF_LEN];

    while(true){
        CONFIDENTIAL uint256_t pin_hash;
        if( !pin_entry(menu, pin_hash, "Set PIN") ){
            return false;
        }
        CONFIDENTIAL uint256_t pin_hash_verify;
        if( !pin_entry(menu, pin_hash_verify, "Verify PIN")){
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


