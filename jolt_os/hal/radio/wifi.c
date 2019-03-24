/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <lwip/sockets.h>
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "lwip/err.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_timer.h"

#include "wifi.h"
#include "vault.h"
#include "jolt_helpers.h"
#include "hal/storage/storage.h"

#if !CONFIG_NO_BLOBS
static const char TAG[] = "wifi_task";

static uint8_t disconnect_ctr = 0;

static void disconnect_timer_cb( void *arg ) {
    if( disconnect_ctr < 15 ){
        disconnect_ctr++;
    }
    else{
        disconnect_ctr = 15;
    }
    esp_wifi_connect();
}

esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    static esp_timer_handle_t disconnect_timer = NULL;
    uint8_t primary;
    wifi_second_chan_t second;
    esp_err_t err;

    if ( NULL == disconnect_timer ) {
        /* Instantiate the battery-saving disconnect timer */
        esp_timer_create_args_t cfg = {
            .callback        = disconnect_timer_cb,
            .arg             = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name            = "wifi_disconn",
        };
        ESP_ERROR_CHECK( esp_timer_create(&cfg, &disconnect_timer) );
    }

    switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGD(TAG, "SYSTEM_EVENT_STA_START");
            disconnect_ctr = 0;
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_STOP:
            /* application event callback generally does not need to do anything */
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            err = esp_wifi_get_channel(&primary, &second);
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP; channel=%d, err=%d, ip: %s\n",
                    primary, err, ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_CONNECTED:
            /* Do nothing, generally need to wait for SYSTEM_EVENT_STA_GOT_IP */
            disconnect_ctr = 0;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED: {
            /* Gets triggered on disconnect, or when esp_wifi_connect() fails to 
             * connect */
            ESP_LOGD(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            if( 0 == disconnect_ctr ){
                /* Try to connect immediately */
                esp_wifi_connect();
                disconnect_ctr = 1;
            }
            else{
                /* Scanning period gradually increases every scan up to 20 seconds.
                 * Idea: Faster, battery hungry scans are more important near the 
                 * first disconnect, but after a while the device is just sitting
                 * idle and doesn't require immediate user feedback. */
                uint64_t timeout;
                timeout = 1000000*disconnect_ctr + 5000000;
                esp_timer_start_once(disconnect_timer, timeout);
            }
            break;
        }
        default:
            break;
    }
    return ESP_OK;
}

esp_err_t jolt_wifi_start(){
    static bool initiate_tcpip_adapter = true;
    wifi_config_t sta_config = {
        .sta = {
            .ssid            = CONFIG_AP_TARGET_SSID,
            .password        = CONFIG_AP_TARGET_PASSWORD,
            .scan_method     = WIFI_FAST_SCAN,
            .bssid_set       = 0,
            .channel         = 0,
            .listen_interval = 3,
            .sort_method     = WIFI_CONNECT_AP_BY_SECURITY,
        }
    };
    
    if ( initiate_tcpip_adapter ) {
        tcpip_adapter_init();
        initiate_tcpip_adapter = false;
    }

    esp_wifi_stop();
    
    /* Check for WiFi credentials in NVS */
    {
        size_t ssid_len;
        storage_get_str(NULL, &ssid_len, "user", "wifi_ssid",
                CONFIG_AP_TARGET_SSID);
        if( ssid_len > 31 ) {
            goto err;
        }
        storage_get_str((char *)sta_config.sta.ssid, &ssid_len,
                "user", "wifi_ssid",
                CONFIG_AP_TARGET_SSID);
    }
    {
        size_t pass_len;
        storage_get_str(NULL, &pass_len, "user", "wifi_pass",
                CONFIG_AP_TARGET_PASSWORD);
        if( pass_len > 63 ) {
            goto err;
        }
        storage_get_str((char *)sta_config.sta.password, &pass_len,
                "user", "wifi_pass",
                CONFIG_AP_TARGET_PASSWORD);
    }
   
    /* Instantiate Drivers */
    {
        esp_err_t err;
        wifi_mode_t mode;
        err = esp_wifi_get_mode( &mode );
        if( ESP_ERR_WIFI_NOT_INIT == err ) {
            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
            ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
            ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
        }
    }

    esp_wifi_start();
    ESP_ERROR_CHECK( esp_wifi_set_ps(WIFI_PS_MAX_MODEM) );

    return ESP_OK;
err:
    return ESP_FAIL;
}

esp_err_t jolt_wifi_stop() {
    esp_err_t err = ESP_OK;

    err = esp_wifi_disconnect();
    switch(err){
        /* These fall through are on purpose */
        case ESP_OK:
            esp_wifi_stop();
        case ESP_ERR_WIFI_NOT_STARTED:
            esp_wifi_set_mode(WIFI_MODE_NULL);
            esp_wifi_deinit();
        case ESP_ERR_WIFI_NOT_INIT:
            /* Do Nothing */
            err = ESP_OK;
            break;
        case ESP_FAIL:
            /* Do Nothing */
            break;
        default:
            /* Do Nothing */
            break;
    }
    return err;
}

void get_ap_info(char * ssid_info, size_t size){
    
    tcpip_adapter_ip_info_t ip;
    memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
    
    wifi_ap_record_t new_ap_info;
    esp_wifi_sta_get_ap_info(&new_ap_info);
    
    if (tcpip_adapter_get_ip_info(ESP_IF_WIFI_STA, &ip) == 0) {
        char ip_address[16];
        snprintf(ip_address, 16, IPSTR, IP2STR(&ip.ip));
        snprintf(ssid_info, size, "SSID: %s\nRSSI: %d\nIP: %s", new_ap_info.ssid, new_ap_info.rssi, ip_address  );
    }
    else{
        snprintf(ssid_info, 20, "Error Not Connected");
    }
    
}


#else
/* Stubs */
esp_err_t jolt_wifi_start() {
    return ESP_OK;
}

esp_err_t jolt_wifi_stop() {
    return ESP_OK;
}

esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    return ESP_OK;
}


void get_ap_info(char * ssid_info, size_t size){
    if(size > 0){
        *ssid_info = '\0';
    }
}

#endif
