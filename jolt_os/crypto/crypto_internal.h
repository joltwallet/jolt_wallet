#ifndef JOLT_CRYPTO_INTERNAL_H__
#define JOLT_CRYPTO_INTERNAL_H__

#include "jolt_crypto.h"

typedef jolt_crypto_status_t ( *jolt_crypto_func_derive_t )( jolt_crypto_t *ctx );

typedef jolt_crypto_status_t ( *jolt_crypto_func_sign_t )( jolt_hash_type_t hash_type, uint8_t *sig, uint16_t sig_len,
                                                           const uint8_t *msg, size_t msg_len,
                                                           const uint8_t *private_key, uint16_t private_key_len );

typedef jolt_crypto_status_t ( *jolt_crypto_func_verify_t )( jolt_hash_type_t hash_type, const uint8_t *sig,
                                                             uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                                             const uint8_t *public_key, uint16_t public_key_len );

typedef jolt_crypto_status_t ( *jolt_crypto_func_init_t )( jolt_crypto_t *ctx );
typedef jolt_crypto_status_t ( *jolt_crypto_func_update_t )( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len );
typedef jolt_crypto_status_t ( *jolt_crypto_func_final_t )( jolt_crypto_t *ctx );

/** Entry in the Jolt Crypto Registry */
typedef struct {
    const char name[16]; /**< NULL terminated string */

    jolt_crypto_func_derive_t derive; /**< Derive public key from private. */

    jolt_crypto_func_init_t sign_init;          /**< Initialize multipart signing */
    jolt_crypto_func_init_t verify_init;        /**< Initialize multipart verifying */

    jolt_crypto_func_update_t sign_update;      /**< Update multipart signing/verifying */
    jolt_crypto_func_update_t verify_update;

    jolt_crypto_func_final_t sign_final;   /**< Finalize multipart signing */
    jolt_crypto_func_final_t verify_final; /**< Finalize multipart verifying */

    uint16_t private_len;   /**< Private key length in bytes. */
    uint16_t public_len;    /**< Public key length in bytes. */
    uint16_t signature_len; /**< Signature length. */
    jolt_hash_type_t default_hash;

} jolt_crypto_entry_t;

extern const jolt_crypto_entry_t jolt_crypto_registry[];

#endif
