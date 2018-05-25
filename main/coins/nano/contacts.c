#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "u8g2.h"
#include "menu8g2.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "nano_lib.h"
#include "../../vault.h"

bool nano_set_contact_public(const uint256_t public_key, const int index){
    /* Returns false on fetch failure */

    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }

    char key[16];
    snprintf(key, sizeof(key), "contact_pub_%d", index);

    nvs_handle h;
    init_nvm_namespace(&h, "nano");
    nvs_set_blob(h, key, public_key, BIN_256);
    nvs_commit(h);
    nvs_close(h);

    return true;
}

bool nano_set_contact_name(const char *buf, const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }
    if(strlen(buf)>CONFIG_JOLT_NANO_CONTACTS_NAME_LEN){
        return false;
    }

    char key[16];
    snprintf(key, sizeof(key), "contact_name_%d", index);

    nvs_handle h;
    init_nvm_namespace(&h, "nano");
    nvs_set_str(h, key, buf);
    nvs_commit(h);
    nvs_close(h);

    return true;
}

bool nano_get_contact_public(uint256_t public_key, const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }
    char key[16];
    snprintf(key, sizeof(key), "contact_pub_%d", index);

    size_t size;
    nvs_handle h;
    init_nvm_namespace(&h, "nano");
    if( ESP_OK != nvs_get_blob(h, key, public_key, &size) ){
        nvs_close(h);
        return false;
    }
    nvs_close(h);
    return true;
}

bool nano_get_contact_name(char *buf, size_t buf_len, const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }
    char key[16];
    snprintf(key, sizeof(key), "contact_name_%d", index);

    size_t size;
    nvs_handle h;
    init_nvm_namespace(&h, "nano");
    if( ESP_OK != nvs_get_str(h, key, NULL, &size) ){
        nvs_close(h);
        return false;
    }

    if(size > buf_len){
        nvs_close(h);
        return false;
    }

    if( ESP_OK != nvs_get_str(h, key, buf, &size) ){
        nvs_close(h);
        return false;
    }

    nvs_close(h);
    return true;
}

void nano_erase_contact(const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }

    nvs_handle h;
    init_nvm_namespace(&h, "nano");

    char key[16];
    snprintf(key, sizeof(key), "contact_pub_%d", index);
    nvs_erase_key(h, key);

    snprintf(key, sizeof(key), "contact_name_%d", index);
    nvs_erase_key(h, key);

    nvs_close(h);
}

