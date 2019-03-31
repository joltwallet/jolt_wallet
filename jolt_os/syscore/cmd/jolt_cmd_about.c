#include "jolt_lib.h"
#include "esp_wifi.h"

extern const jolt_version_t JOLT_OS_VERSION;   /**< JoltOS version */
extern const jolt_version_t JOLT_JELF_VERSION; /**< Used to determine app compatibility */
extern const jolt_version_t JOLT_HW_VERSION;   /**< To check hardware compatability */


int jolt_cmd_about(int argc, char** argv) {
    if( 1 == argc ) {
        printf("Hardware: %d.%d.%d\n", JOLT_HW_VERSION.major, JOLT_HW_VERSION.minor, JOLT_HW_VERSION.patch);
        printf("JoltOS: %d.%d.%d %s\n", JOLT_OS_VERSION.major, JOLT_OS_VERSION.minor, JOLT_OS_VERSION.patch, JOLT_OS_COMMIT);
        printf("JELF Loader: %d.%d.%d\n", JOLT_JELF_VERSION.major, JOLT_JELF_VERSION.minor, JOLT_JELF_VERSION.patch);
        return 0;
    }

    if( !console_check_equal_argc(argc, 2) ) {
        goto exit;
    }

    if( 0 == strcmp(argv[1], "wifi") ) {
        char ssid[32];
        {
            size_t ssid_len;
            storage_get_str(NULL, &ssid_len, "user", "wifi_ssid",
                    CONFIG_AP_TARGET_SSID);
            if( ssid_len > 31 ) {
                goto exit;
            }
            storage_get_str(ssid, &ssid_len,
                    "user", "wifi_ssid",
                    CONFIG_AP_TARGET_SSID);
        }
        printf("SSID: %s\n", ssid);

#if !CONFIG_NO_BLOBS
        wifi_mode_t mode;
        esp_err_t err = esp_wifi_get_mode(&mode);
        if( ESP_OK == err ) {
            wifi_ap_record_t ap_info;
            if(esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK) {
                printf("Wifi Disconnected\n");
            }
            else {
                printf("RSSI: %d", ap_info.rssi);
            }
        }
        else {
            printf("Wifi Disconnected\n");
        }
        return 0;
#else
        printf("No wifi drivers installed.\n");
#endif
    }

exit:
    return -1;
}
