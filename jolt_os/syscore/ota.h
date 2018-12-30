#ifndef JOLT_SYSCORE_OTA_H__
#define JOLT_SYSCORE_OTA_H__

#include "esp_ota_ops.h"

void jolt_ota_get_partition_table_hash( uint256_t hash );
void jolt_ota_get_bootloader_hash( uint256_t hash );

esp_err_t jolt_ota_init_handle( );
esp_err_t jolt_ota_get_handle( esp_ota_handle_t *ota_handle );
esp_err_t jolt_ota_ymodem(int8_t *progress);

#endif
