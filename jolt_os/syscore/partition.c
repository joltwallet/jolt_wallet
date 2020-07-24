//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "partition.h"
#include "esp_flash_partitions.h"
#include "esp_log.h"
#include "esp_partition.h"
#include "esp_system.h"
#include "sodium.h"
#include "string.h"

static const char TAG[] = "syscore/partition";

static const char* const APPROVED_TABLES[] = {
        "aeffaa190e6132043239c52df035a6546bbd07b44d6f44ee872b708c15587a8c",
};

static const char* const APPROVED_BOOTLOADERS[] = {
        "8a9cf142b9a3833b3cbdcfe2404abf0c2edbea8469500eca6b696d4998857fec",
};

void jolt_partition_get_table_hash( uint256_t hash )
{
    esp_partition_t partition;

    /* get sha256 digest for the partition table */
    partition.address = ESP_PARTITION_TABLE_OFFSET;
    partition.size    = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type    = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256( &partition, hash );

    {
        char hex[HEX_256];
        sodium_bin2hex( hex, sizeof( hex ), hash, sizeof( uint256_t ) );
        ESP_LOGI( TAG, "Partition Table SHA256: %s", hex );
    }
}

void jolt_partition_get_bootloader_hash( uint256_t hash )
{
    /* get sha256 digest for bootloader */
    esp_partition_t partition;
    partition.address = ESP_BOOTLOADER_OFFSET;
    partition.size    = ESP_PARTITION_TABLE_OFFSET;
    partition.type    = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256( &partition, hash );

    {
        char hex[HEX_256];
        sodium_bin2hex( hex, sizeof( hex ), hash, sizeof( uint256_t ) );
        ESP_LOGI( TAG, "Bootloader SHA256: %s", hex );
    }
}

void jolt_partition_check_table( void )
{
    uint256_t hash = {0};
    jolt_partition_get_table_hash( hash );
    for( uint8_t i = 0; i < sizeof( APPROVED_TABLES ); i++ ) {
        uint256_t approved_bin;
        ESP_ERROR_CHECK(
                sodium_hex2bin( approved_bin, sizeof( approved_bin ), APPROVED_TABLES[i], 64, NULL, NULL, NULL ) );
        if( 0 == memcmp( approved_bin, hash, sizeof( approved_bin ) ) ) {
            ESP_LOGD( TAG, "Table hash matched %d bytes with approved index %d", sizeof( approved_bin ), i );
            return;
        }
    }
    ESP_LOGE( TAG, "Invalid Partition Table Hash" );
    esp_restart();
}

void jolt_partition_check_bootloader( void )
{
    uint256_t hash = {0};
    jolt_partition_get_bootloader_hash( hash );
    for( uint8_t i = 0; i < sizeof( APPROVED_BOOTLOADERS ); i++ ) {
        uint256_t approved_bin;
        ESP_ERROR_CHECK( sodium_hex2bin( approved_bin, sizeof( approved_bin ), APPROVED_BOOTLOADERS[i], 64, NULL, NULL,
                                         NULL ) );
        if( 0 == memcmp( approved_bin, hash, sizeof( approved_bin ) ) ) {
            ESP_LOGD( TAG, "Bootloader hash matched %d bytes with approved index %d", sizeof( approved_bin ), i );
            return;
        }
    }
    ESP_LOGE( TAG, "Invalid Bootloader Hash" );
    esp_restart();
}
