#ifndef JOLT_HASH_INTERNAL_H__
#define JOLT_HASH_INTERNAL_H__

#include "jolt_hash.h"

typedef jolt_hash_status_t ( *jolt_hash_func_single_t )( uint8_t *hash, size_t hash_len, const uint8_t *msg,
                                                         size_t msg_len, const uint8_t *key, uint8_t key_len );

typedef jolt_hash_status_t ( *jolt_hash_func_init_t )( jolt_hash_t *ctx );
typedef jolt_hash_status_t ( *jolt_hash_func_update_t )( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len );
typedef jolt_hash_status_t ( *jolt_hash_func_final_t )( jolt_hash_t *ctx );

typedef struct {
    const char name[15]; /**< NULL terminated string */
    struct {
        uint8_t min; /**< Minimum output buffer size (inclusive) */
        uint8_t max; /**< Maximum output buffer size (inclusive) */
    } size;

    jolt_hash_func_single_t single; /**< Function for single-pass hash */

    jolt_hash_func_init_t init;     /**< Initialize multipart hashing */
    jolt_hash_func_update_t update; /**< Update multipart hashing */
    jolt_hash_func_final_t final;   /**< Finalize multipart hashing */

} jolt_hash_entry_t;

extern const jolt_hash_entry_t jolt_hash_registry[];

#endif
