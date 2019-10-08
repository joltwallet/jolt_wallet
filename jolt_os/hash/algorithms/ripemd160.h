#ifndef JOLT_HASH_RIPEMD160_H__
#define JOLT_HASH_RIPEMD160_H__

#include "jolt_hash.h"

jolt_hash_status_t jolt_hash_ripemd160( uint8_t *hash, size_t hash_len, const uint8_t *msg, size_t msg_len,
                                        const uint8_t *key, uint8_t key_len );

jolt_hash_status_t jolt_hash_ripemd160_init( jolt_hash_t *ctx );

jolt_hash_status_t jolt_hash_ripemd160_update( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len );

jolt_hash_status_t jolt_hash_ripemd160_final( jolt_hash_t *ctx );

#endif
