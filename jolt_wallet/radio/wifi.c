/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

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
#include "lwip/err.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "wifi.h"
#include "../vault.h"
#include "../helpers.h"
#include "../hal/storage.h"

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
            ESP_LOGI(TAG, "channel=%d err=%d\n", primary, err);
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "got ip:%s\n",
                     ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGD(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            break;
    }
    return ESP_OK;
}

void wifi_connect(){
    wifi_config_t sta_config = {
        .sta = {
            .ssid      = CONFIG_AP_TARGET_SSID,
            .password  = CONFIG_AP_TARGET_PASSWORD,
            .bssid_set = 0
        }
    };
    
    tcpip_adapter_init();
    
    //Check for WiFi credentials in NVS
    size_t string_size_ssid;
    storage_get_str(NULL, &string_size_ssid, "user", "wifi_ssid",
            CONFIG_AP_TARGET_SSID);
    char* wifi_ssid = malloc(string_size_ssid);
    storage_get_str(wifi_ssid, &string_size_ssid, "user", "wifi_ssid",
            CONFIG_AP_TARGET_SSID);

    size_t string_size_pass;
    storage_get_str(NULL, &string_size_pass, "user", "wifi_pass",
            CONFIG_AP_TARGET_PASSWORD);
    char* wifi_pass = malloc(string_size_pass);
    storage_get_str(wifi_pass, &string_size_pass, "user", "wifi_pass",
            CONFIG_AP_TARGET_PASSWORD);
  
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    strcpy((char *)sta_config.sta.ssid, (char *)wifi_ssid);
    strcpy((char *)sta_config.sta.password, (char *)wifi_pass);
   
#if 0
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
#endif
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
    //return 0; //todo: esp_wifi_sta_get_ap_info reboots on virgin boot
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

bool set_wifi_credentials(char *ssid, char *pass) {
    /* Requires a reboot for credentials to take effect */
    if( !vault_refresh() ) {
        return false;
    }
    storage_set_str(ssid, "user", "wifi_ssid");
    storage_set_str(pass, "user", "wifi_pass");

    return true;
}
