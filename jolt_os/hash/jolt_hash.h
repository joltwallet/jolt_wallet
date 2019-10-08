#ifndef JOLT_HASH_H__
#define JOLT_HASH_H__

#include "sodium.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdint.h"

/* Unified interface to use built-in hashing functions */

enum {
    JOLT_HASH_STATUS_SUCCESS = 0, /**< Hash computation successful */
    JOLT_HASH_STATUS_FAIL,        /**< Generic failure */
    JOLT_HASH_STATUS_NOT_IMPL,    /**< Functionality not implemented yet */
    JOLT_HASH_STATUS_PARAM,       /**< Invalid input parameters */
    JOLT_HASH_STATUS_OOM,         /**< Ran out of memory */
    JOLT_HASH_STATUS_INSUFF_BUF,  /**< Output buffer is too small */
    JOLT_HASH_STATUS_NOT_INIT,    /**< Current hashing state is not initialized */
};
typedef uint8_t jolt_hash_status_t;

enum {
    JOLT_HASH_UNDEFINED = 0,
    JOLT_HASH_BLAKE2B,
    JOLT_HASH_SHA256,
    JOLT_HASH_MAX_IDX,
};
typedef uint8_t jolt_hash_type_t;

typedef struct {
    uint8_t key[64];
    uint8_t *hash;
    void *state; /**< Store state for specific hash algorithm */

    jolt_hash_type_t type;
    uint8_t key_len;
    uint8_t hash_len;
    uint8_t initialized : 1;
} jolt_hash_t;

/**
 * @brief
 * @param[in] type Enumerated hashing algorith identifier
 * @param[out] hash Output hash buffer.
 * @param[in] hash_len Output hash buffer size.
 * @param[in] msg Data to hash.
 * @param[in] msg_len Amount of data to hash.
 * @param[in] key [Optional] hashing key. NULL if not used.
 * @param[in] key_len [Optional] hashing key length. 0 if not used.
 * @return Jolt hashing status code.
 */
jolt_hash_status_t jolt_hash( jolt_hash_type_t type, uint8_t *hash, size_t hash_len, const uint8_t *msg,
                              size_t msg_len, const uint8_t *key, uint8_t key_len );

/**
 * @brief Initialize a jolt hashing context.
 *
 * Prior to calling this function, you must fill out the following fields of
 * the ctx object:
 *     1. ``key`` and ``key_len``. If not used, set key_len to 0. Key can have a
 *     maximum length of 64 bytes. Not all hashing functions allow a key.
 *     2. ``hash`` and ``hash_len``. Have this point to a buffer to store
 *     intermediate and final results.
 *     3. ``type`` enum for hashing algorithm to use.
 *
 * @param[in, out] ctx Jolt hashing context.
 * @param[in] type Hashing algorithm to use.
 * @return Jolt hashing status code.
 */

jolt_hash_status_t jolt_hash_init( jolt_hash_t *ctx );

/**
 * @brief Add data to the hash digest.
 * @param[in] ctx Jolt hashing context.
 * @param[in] msg Data to hash
 * @param[in] msg_len Number of bytes to hash from ``msg``.
 * @return Jolt hashing status code.
 */
jolt_hash_status_t jolt_hash_update( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len );

/**
 * @brief Finalizes the hash and frees any memory allocated by ctx.
 *
 * Will explicitly zero out all data.
 *
 * @param[in] ctx Jolt hashing context.
 * @return Jolt hashing status code.
 */
jolt_hash_status_t jolt_hash_final( jolt_hash_t *ctx );

/**
 * @brief Converts a hash name-string to a valid jolt identifier.
 * @param[in] name Hash name. Gets matched to the `name` field in the Jolt Hash Registry.
 * @return Hash type. Returns `JOLT_HASH_UNDEFINED` (0) if not found.
 */
jolt_hash_type_t jolt_hash_from_str( const char *name );

#endif
