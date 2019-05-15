#include "jolt_lib.h"
#include "esp_wifi.h"

extern const jolt_version_t JOLT_OS_VERSION;   /**< JoltOS version */
extern const jolt_version_t JOLT_JELF_VERSION; /**< Used to determine app compatibility */
extern const jolt_version_t JOLT_HW_VERSION;   /**< To check hardware compatability */

#define PRINT_AND_END(x) if(print_response(x)) return_code = 0; else return_code=1; goto end;
/**
 * @brief Adds a json node with key and semver value
 * @param[in] json parenting json node to add to
 * @param[in] key key to insert the node into json as
 * @return True on success, False on failure
 */
static bool add_semver_to_object( cJSON *json, const char *key, uint8_t major, uint8_t minor, uint8_t patch ) {
    char buf[20];
    snprintf(buf, sizeof(buf), "%d.%d.%d", major, minor, patch);

    if( NULL == cJSON_AddStringToObject(json, key, buf) ) goto end;

    return true;
end:
    return false;
}

static bool print_response(cJSON * json) {
    char *response = NULL;
    response = cJSON_Print(json);
    if( NULL == response ) return false;
    printf(response);
    printf("\n");
    free(response);
    return true;
}


int jolt_cmd_about(int argc, char** argv) {
    int return_code = -1;
    cJSON *json = NULL;

    if( NULL == (json = cJSON_CreateObject()) ) goto end;

    if( 1 == argc ) {
        /* General "about" information */
        if( !add_semver_to_object(json, "hardware", JOLT_HW_VERSION.major, JOLT_HW_VERSION.minor, JOLT_HW_VERSION.patch)) goto end;
        if( !add_semver_to_object(json, "jolt_os", JOLT_OS_VERSION.major, JOLT_OS_VERSION.minor, JOLT_OS_VERSION.patch)) goto end;
        if( NULL == cJSON_AddStringToObject(json, "jolt_os_commit", JOLT_OS_COMMIT)) goto end; 
        if( !add_semver_to_object(json, "jelf_loader", JOLT_JELF_VERSION.major, JOLT_JELF_VERSION.minor, JOLT_JELF_VERSION.patch)) goto end;
        PRINT_AND_END(json);
    }

    if( !console_check_equal_argc(argc, 2) ) {
        goto end;
    }

    if( 0 == strcmp(argv[1], "wifi") ) {
        char ssid[32];
        {
            size_t ssid_len;
            storage_get_str(NULL, &ssid_len, "user", "wifi_ssid",
                    CONFIG_AP_TARGET_SSID);
            if( ssid_len > 31 ) {
                goto end;
            }
            storage_get_str(ssid, &ssid_len,
                    "user", "wifi_ssid",
                    CONFIG_AP_TARGET_SSID);
        }
        if( NULL == cJSON_AddStringToObject(json, "ssid", ssid) ) goto end;

        /* Get strength */
#if !CONFIG_NO_BLOBS
        {
            wifi_mode_t mode;
            esp_err_t err = esp_wifi_get_mode(&mode);
            int rssi = 0;
            if( ESP_OK == err ) {
                wifi_ap_record_t ap_info;
                if(esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
                    rssi = ap_info.rssi;
                }
            }
            if(NULL == cJSON_AddNumberToObject(json, "rssi", rssi)) goto end;
            PRINT_AND_END(json);
        }
#else
        {
            if(NULL == cJSON_AddNumberToObject(json, "rssi", 0)) goto end;
            PRINT_AND_END(json);
        }
#endif
    }

end:
    if( NULL != json ) cJSON_Delete(json);
    return return_code;
}
