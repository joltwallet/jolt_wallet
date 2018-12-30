
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "string.h"

#include "jolttypes.h"
#include "ymodem.h"

#include "nvs.h"
#include "nvs_flash.h"

#include "ota.h"

static const char TAG[] = "jolt_ota";

static esp_ota_handle_t jolt_ota_handle = 0; // underlying uint32_t
static esp_partition_t *update_partition = NULL;

/* Static Function Declaration */
static int ota_ymodem_write_wrapper(const void *data, 
        int32_t size, int32_t nmemb, esp_ota_handle_t cookie);

static void jolt_ota_clear_globals();
static void jolt_ota_ymodem_task( void *param );


void jolt_ota_get_partition_table_hash( uint256_t hash ) {
    esp_partition_t partition;

    // get sha256 digest for the partition table
    partition.address   = ESP_PARTITION_TABLE_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type      = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, hash);
}

void jolt_ota_get_bootloader_hash( uint256_t hash ) {
    /* get sha256 digest for bootloader */
    esp_partition_t partition;
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, hash);
}

static int ota_ymodem_write_wrapper(const void *data_buf, 
        int32_t size, int32_t nmemb, esp_ota_handle_t cookie) {
    // todo: error handling
    size *= nmemb;
    esp_ota_write( cookie, data_buf, size );
    return nmemb;
}

/* Clears global variables */
static void jolt_ota_clear_globals() {
    if( 0 != jolt_ota_handle ) {
        if (esp_ota_end(jolt_ota_handle) != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_end failed!");
        }
        else {
            jolt_ota_handle = 0;
        }
    }
    if( NULL != update_partition){
        update_partition = NULL;
    }
}

esp_err_t jolt_ota_ymodem(int8_t *progress) {
    /* Performs OTA update over Ymodem */
    esp_err_t err = ESP_FAIL;

    /**********************************
     * Get/Init Handle of OTA Process *
     **********************************/
    err = jolt_ota_init_handle();
    if( ESP_OK != err ){
        goto exit;
    }

    /***************************************
     * Receive and Write data to partition *
     ***************************************/
    size_t binary_file_length;
    binary_file_length = Ymodem_Receive_Write(jolt_ota_handle,
            update_partition->size, NULL, &ota_ymodem_write_wrapper, progress);
    if( binary_file_length <= 0 ) {
        ESP_LOGE(TAG, "Error during firmware transfer.");
        goto exit;
    }
    ESP_LOGI(TAG, "Total Write binary data length : %d", binary_file_length);

    /*****************************
     * Close the jolt_ota_handle *
     *****************************/
    //ota_ymodem_write_wrapper(NULL, 1, -1, jolt_ota_handle); 
    if (esp_ota_end(jolt_ota_handle) != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end failed!");
        jolt_ota_handle = 0;
        goto exit;
    }
    else {
        jolt_ota_handle = 0;
    }

    /*********************
     * Post-Flight Check *
     *********************/
    if (esp_partition_check_identity(esp_ota_get_running_partition(), update_partition) == true) {
        ESP_LOGI(TAG, "The current running firmware is same as the firmware just downloaded");
    }

    /**********************************************
     * Set the update_partition as boot partition *
     **********************************************/
    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        goto exit;
    }

    ESP_LOGI(TAG, "OTA Complete. Please Reboot System");

    err = ESP_OK;

exit:
    return err;
}

esp_err_t jolt_ota_init_handle( ) {
    /* OTA Firmware Update Main Abstraction 
     *     * Starts the OTA process.
     * Generic to WiFi/Ymodem method */
    esp_err_t err = ESP_FAIL;

    if( 0 != jolt_ota_handle || NULL != update_partition){
        ESP_LOGW(TAG, "OTA already in prgoress");
        goto exit;
    }

    ESP_LOGI(TAG, "Starting OTA Firmware Update...");

    /*******************************
     * Run Some System Diagnostics *
     *******************************/
    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running    = esp_ota_get_running_partition();

    if (configured != running) {
        ESP_LOGW(TAG, "Configured OTA boot partition at offset 0x%08x, "
                "but running from offset 0x%08x",
                 configured->address, running->address);
        ESP_LOGW(TAG, "(This can happen if either the OTA boot data or "
                "preferred boot image become corrupted somehow.)");
    }
    ESP_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)",
            running->type, running->subtype, running->address);

    /**********************************
     * Find Partition to Write Update *
     **********************************/
    update_partition = esp_ota_get_next_update_partition(NULL);
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
            update_partition->subtype, update_partition->address);
    assert(update_partition != NULL);

    /**********************************
     * Find Partition to Write Update *
     **********************************/
    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &jolt_ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        goto exit;
    }
    err = ESP_OK;

exit:
    return err;
}
