#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "hash_internal.h"

jolt_hash_status_t jolt_hash_blake2b( uint8_t *hash, size_t hash_len, const uint8_t *msg, size_t msg_len,
                                      const uint8_t *key, uint8_t key_len )
{
    return JOLT_HASH_STATUS_NOT_IMPL;
}

jolt_hash_status_t jolt_hash_blake2b_init( jolt_hash_t *ctx ) { return JOLT_HASH_STATUS_NOT_IMPL; }

jolt_hash_status_t jolt_hash_blake2b_update( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len )
{
    return JOLT_HASH_STATUS_NOT_IMPL;
}

jolt_hash_status_t jolt_hash_blake2b_final( jolt_hash_t *ctx ) { return JOLT_HASH_STATUS_NOT_IMPL; }
