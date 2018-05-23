#include <lwip/sockets.h>
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "wifi.h"
#include "lwip/err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "vault.h"

static const char *TAG = "wifi_task";

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    uint8_t primary;
    wifi_second_chan_t second;
    esp_err_t err;
    
    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            err = esp_wifi_get_channel(&primary, &second);
            printf("channel=%d err=%d\n", primary, err);
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "got ip:%s\n",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi_connect(){
    nvs_handle wifi_nvs_handle;
    wifi_config_t sta_config = {
        .sta = {
            .ssid      = CONFIG_AP_TARGET_SSID,
            .password  = CONFIG_AP_TARGET_PASSWORD,
            .bssid_set = 0
        }
    };
    
    nvs_flash_init();
    tcpip_adapter_init();
    
    //Check for WiFi credentials in NVS
    init_nvm_namespace(&wifi_nvs_handle, "user");
    
    size_t string_size_ssid;
    size_t string_size_pass;
    esp_err_t err = nvs_get_str(wifi_nvs_handle, "wifi_ssid", NULL, &string_size_ssid);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "WiFi SSID not found, setting default");
        nvs_set_str(wifi_nvs_handle, "wifi_ssid", CONFIG_AP_TARGET_SSID);
        nvs_set_str(wifi_nvs_handle, "wifi_pass", CONFIG_AP_TARGET_PASSWORD);
        err = nvs_commit(wifi_nvs_handle);
    }

    ESP_LOGI(TAG, "WiFi SSID found, loading up");
    char* wifi_ssid = malloc(string_size_ssid);
    err = nvs_get_str(wifi_nvs_handle, "wifi_ssid", wifi_ssid, &string_size_ssid);
    
    err = nvs_get_str(wifi_nvs_handle, "wifi_pass", NULL, &string_size_pass);
    char* wifi_pass = malloc(string_size_pass);
    err = nvs_get_str(wifi_nvs_handle, "wifi_pass", wifi_pass, &string_size_pass);
    
    nvs_close(wifi_nvs_handle);
    
    
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    strcpy((char *)sta_config.sta.ssid, (char *)wifi_ssid);
    strcpy((char *)sta_config.sta.password, (char *)wifi_pass);
    
    ESP_LOGI(TAG, "wifi_ssid: %s\n"
            "wifi_pass: %s\n"
            "strlen(wifi_ssid): %d\n"
            "strlen(wifi_pass): %d\n"
            "string_size_ssid: %d\n"
            "string_size_pass: %d\n"
            "sta_config.sta.ssid: %s\n"
            "sta_config.sta.password: %s\n"
            "sizeof(sta_config.sta.ssid): %d\n"
            "sizeof(sta_config.sta.pass): %d\n", 
            wifi_ssid, 
            wifi_pass, 
            strlen(wifi_ssid), 
            strlen(wifi_pass), 
            string_size_ssid, 
            string_size_pass, 
            sta_config.sta.ssid, 
            sta_config.sta.password, 
            sizeof(sta_config.sta.ssid), 
            sizeof(sta_config.sta.password));
    
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    //ESP_ERROR_CHECK(esp_wifi_connect());
}

uint8_t get_wifi_strength(){
    /* 0 - no wifi
     * 1 - weak
     * 2 - medium
     * 3 - strong
     */
    wifi_ap_record_t ap_info;
    if(esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK){
        return 0;
    }

    if (ap_info.rssi >= -55) {
        return 3;
    }
    else if (ap_info.rssi >= -75) {
        return 2;
    }
    else {
        return 1;
    }
}

void get_ap_info(char * ssid_info, size_t size){
    
    tcpip_adapter_ip_info_t ip;
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    
    wifi_ap_record_t new_ap_info;
    esp_wifi_sta_get_ap_info(&new_ap_info);
    
    if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0) {
        char ip_address[16];
        snprintf(ip_address, 16, IPSTR, IP2STR(&ip.ip));
        snprintf(ssid_info, size, "SSID: %s RSSI:%d IP:%s", new_ap_info.ssid, new_ap_info.rssi, ip_address  );
    }
    else{
        snprintf(ssid_info, 20, "Error Not Connected");
    }
    
}
