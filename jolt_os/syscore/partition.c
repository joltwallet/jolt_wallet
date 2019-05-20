//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "esp_log.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "sodium.h"

#include "partition.h"

static const char TAG[] = "syscore/partition";


void jolt_partition_get_table_hash( uint256_t hash ) {
    esp_partition_t partition;

    /* get sha256 digest for the partition table */
    partition.address   = ESP_PARTITION_TABLE_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type      = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, hash);

    {
        char hex[HEX_256];
        sodium_bin2hex( hex, sizeof(hex), hash, sizeof(uint256_t) );
        ESP_LOGI(TAG, "Partition Table SHA256: %s", hex);
    }
}

void jolt_partition_get_bootloader_hash( uint256_t hash ) {
    /* get sha256 digest for bootloader */
    esp_partition_t partition;
    partition.address   = ESP_BOOTLOADER_OFFSET;
    partition.size      = ESP_PARTITION_TABLE_OFFSET;
    partition.type      = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, hash);

    {
        char hex[HEX_256];
        sodium_bin2hex( hex, sizeof(hex), hash, sizeof(uint256_t) );
        ESP_LOGI(TAG, "Bootloader SHA256: %s", hex);
    }
}
