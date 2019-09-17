#include "esp_spi_flash.h"
#include "esp_system.h"
#include "hal/radio/wifi.h"
#include "jolt_lib.h"
#include "syscore/filesystem.h"

#if CONFIG_JOLT_STORE_ATAES132A
    #include "aes132_cmd.h"
#endif

static const char TAG[] = "jolt_cmd_about";

extern const jolt_version_t JOLT_OS_VERSION;   /**< JoltOS version */
extern const jolt_version_t JOLT_JELF_VERSION; /**< Used to determine app compatibility */
extern const jolt_version_t JOLT_HW_VERSION;   /**< To check hardware compatability */

#define PRINT_AND_END( x )    \
    if( print_response( x ) ) \
        return_code = 0;      \
    else                      \
        return_code = 1;      \
    goto end;

/**
 * @brief Adds a json node with key and semver value
 * @param[in] json parenting json node to add to
 * @param[in] key key to insert the node into json as
 * @return True on success, False on failure
 */
static bool add_semver_to_object( cJSON *json, const char *key, uint8_t major, uint8_t minor, uint8_t patch )
{
    char buf[20];
    snprintf( buf, sizeof( buf ), "%d.%d.%d", major, minor, patch );

    if( NULL == cJSON_AddStringToObject( json, key, buf ) ) goto end;

    return true;
end:
    return false;
}

static bool print_response( cJSON *json )
{
    char *response;
    response = cJSON_Print( json );
    if( NULL == response ) return false;
    printf( response );
    printf( "\n" );
    free( response );
    return true;
}

int jolt_cmd_about( int argc, char **argv )
{
    int return_code = -1;
    cJSON *json     = NULL;

    if( NULL == ( json = cJSON_CreateObject() ) ) {
        ESP_LOGE( TAG, "Unable to allocate cJSON object." );
        goto end;
    }

    if( 1 == argc ) {
        /* General "about" information */

        {
            /* Chip Info */
            cJSON *chip_json = NULL;
            esp_chip_info_t chip_info;
            uint16_t flash_size_mb = spi_flash_get_chip_size() / ( 1024 * 1024 );
            esp_chip_info( &chip_info );
            if( NULL == ( chip_json = cJSON_AddObjectToObject( json, "chip" ) ) ) goto end;
            if( NULL == cJSON_AddNumberToObject( chip_json, "revision", chip_info.revision ) ) goto end;
            if( NULL == cJSON_AddNumberToObject( chip_json, "cores", chip_info.cores ) ) goto end;
            if( NULL == cJSON_AddNumberToObject( chip_json, "flash_size_mb", flash_size_mb ) ) goto end;
            if( NULL == cJSON_AddBoolToObject( chip_json, "wifi_bgn", chip_info.features & CHIP_FEATURE_WIFI_BGN ) )
                goto end;
            if( NULL == cJSON_AddBoolToObject( chip_json, "ble", chip_info.features & CHIP_FEATURE_BLE ) ) goto end;
        }

#if CONFIG_JOLT_STORE_ATAES132A
        /* ATAES132A Info */
        do {
            cJSON *chip_json = NULL;
            if( NULL == ( chip_json = cJSON_AddObjectToObject( json, "ataes132a" ) ) ) goto end;
            bool locked = false;
            if( aes132_check_configlock( &locked ) ) {
                ESP_LOGE( TAG, "Unable to check ATAES132A lock status." );
                cJSON_Delete( chip_json );
                break;
            }
            if( NULL == cJSON_AddBoolToObject( chip_json, "locked", locked ) ) goto end;
        } while( 0 );
#endif

        if( !add_semver_to_object( json, "hardware", JOLT_HW_VERSION.major, JOLT_HW_VERSION.minor,
                                   JOLT_HW_VERSION.patch ) )
            goto end;
        if( !add_semver_to_object( json, "jolt_os", JOLT_OS_VERSION.major, JOLT_OS_VERSION.minor,
                                   JOLT_OS_VERSION.patch ) )
            goto end;
        if( NULL == cJSON_AddStringToObject( json, "jolt_os_commit", JOLT_OS_COMMIT ) ) goto end;
        if( !add_semver_to_object( json, "jelf_loader", JOLT_JELF_VERSION.major, JOLT_JELF_VERSION.minor,
                                   JOLT_JELF_VERSION.patch ) )
            goto end;
    }
    else if( argc < 2 ) {
        goto end;
    }
    else if( 0 == strcmp( argv[1], "wifi" ) ) {
        {
            char ssid[JOLT_WIFI_SSID_MAX_LEN + 1] = {0};
            size_t ssid_len;
            storage_get_str( NULL, &ssid_len, "user", "wifi_ssid", CONFIG_AP_TARGET_SSID );
            if( ssid_len > JOLT_WIFI_SSID_MAX_LEN ) goto end;
            storage_get_str( ssid, &ssid_len, "user", "wifi_ssid", CONFIG_AP_TARGET_SSID );
            if( NULL == cJSON_AddStringToObject( json, "ssid", ssid ) ) goto end;
        }

        /* Get strength */
        {
            int8_t rssi;
            rssi = jolt_wifi_get_rssi();
            if( NULL == cJSON_AddNumberToObject( json, "rssi", rssi ) ) goto end;
        }

        /* Get IP Address */
        {
            char *ip;
            ip = jolt_wifi_get_ip();
            if( NULL == ip ) {
                if( NULL == cJSON_AddStringToObject( json, "ip", EMPTY_STR ) ) goto end;
            }
            else {
                if( NULL == cJSON_AddStringToObject( json, "ip", ip ) ) goto end;
            }
            free( ip );
        }
    }
    else if( 0 == strcmp( argv[1], "filesystem" ) ) {
        size_t total_bytes = 0, used_bytes = 0;
        if( ESP_OK != jolt_fs_info( &total_bytes, &used_bytes ) ) goto end;

        if( NULL == cJSON_AddStringToObject( json, "type", jolt_fs_type() ) ) goto end;
        if( NULL == cJSON_AddNumberToObject( json, "total", total_bytes ) ) goto end;
        if( NULL == cJSON_AddNumberToObject( json, "used", used_bytes ) ) goto end;
    }

    PRINT_AND_END( json );

end:
    if( NULL != json ) cJSON_Delete( json );
    return return_code;
}
