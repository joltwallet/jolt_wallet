#include "jolt_lib.h"
#include "hal/radio/wifi.h"
#include "syscore/filesystem.h"

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
    }
    else if( argc < 2 ) {
        goto end;
    }
    else if( 0 == strcmp(argv[1], "wifi") ) {
        {
            char ssid[SSID_MAX_LEN+1] = { 0 };
            size_t ssid_len;
            storage_get_str(NULL, &ssid_len, "user", "wifi_ssid",
                    CONFIG_AP_TARGET_SSID);
            if( ssid_len > SSID_MAX_LEN ) goto end;
            storage_get_str(ssid, &ssid_len,
                    "user", "wifi_ssid",
                    CONFIG_AP_TARGET_SSID);
            if( NULL == cJSON_AddStringToObject(json, "ssid", ssid) ) goto end;
        }

        /* Get strength */
        {
            int8_t rssi;
            rssi = jolt_wifi_get_rssi();
            if(NULL == cJSON_AddNumberToObject(json, "rssi", rssi)) goto end;
        }

        /* Get IP Address */
        {
            char *ip = NULL;
            ip = jolt_wifi_get_ip();
            if(NULL == ip){
                if( NULL == cJSON_AddStringToObject(json, "ip", EMPTY_STR) ) goto end;
            }
            else{
                if( NULL == cJSON_AddStringToObject(json, "ip", ip) ) goto end;
            }
            free(ip);
        }
    }
    else if( 0 == strcmp(argv[1], "filesystem")) {
        size_t total_bytes = 0, used_bytes = 0;
        if( ESP_OK != jolt_fs_info(&total_bytes, &used_bytes) ) goto end;

        if( NULL == cJSON_AddStringToObject(json, "type", jolt_fs_type()) ) goto end;
        if(NULL == cJSON_AddNumberToObject(json, "total", total_bytes)) goto end;
        if(NULL == cJSON_AddNumberToObject(json, "used", used_bytes)) goto end;
    }

    PRINT_AND_END(json);

end:
    if( NULL != json ) cJSON_Delete(json);
    return return_code;
}
