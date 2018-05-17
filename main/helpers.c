#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"

#include "u8g2.h"
#include "nano_lib.h"
#include "nvs_flash.h"
#include "nvs.h"


static const char* TAG = "helpers";

uint8_t get_center_x(u8g2_t *u8g2, const char *text){
    // Computes X position to print text in center of screen
    u8g2_uint_t width = u8g2_GetStrWidth(u8g2, text);
    return (u8g2_GetDisplayWidth(u8g2)-width)/2 ;
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

