#ifndef JOLT_SYSCORE_PARTITION_H__
#define JOLT_SYSCORE_PARTITION_H__

#include "jolttypes.h"

/**
 * @brief Get the hash of the partition table
 * @param[out] 256-bit Blake2b hash of partition table
 */
void jolt_partition_get_table_hash( uint256_t hash );

/**
 * @brief Get the hash of the bootloader
 * @param[out] 256-bit Blake2b hash of partition table
 */
void jolt_partition_get_bootloader_hash( uint256_t hash );


#endif
