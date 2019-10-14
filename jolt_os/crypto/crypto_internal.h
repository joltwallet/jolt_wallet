#ifndef JOLT_CRYPTO_INTERNAL_H__
#define JOLT_CRYPTO_INTERNAL_H__

#include "jolt_crypto.h"

typedef jolt_crypto_status_t ( *jolt_crypto_func_derive_t )( uint8_t *public_key, uint16_t public_key_len,
                                                             const uint8_t *private_key, uint16_t private_key_len );

typedef jolt_crypto_status_t ( *jolt_crypto_func_sign_t )( uint8_t *sig, uint16_t sig_len, const uint8_t *msg,
                                                           size_t msg_len, const uint8_t *public_key,
                                                           uint16_t public_key_len, const uint8_t *private_key,
                                                           uint16_t private_key_len );

typedef jolt_crypto_status_t ( *jolt_crypto_func_verify_t )( const uint8_t *sig, uint16_t sig_len, const uint8_t *msg,
                                                             size_t msg_len, const uint8_t *public_key,
                                                             uint16_t public_key_len );

/** Entry in the Jolt Crypto Registry */
typedef struct {
    const char name[20]; /**< NULL terminated string */

    jolt_crypto_func_derive_t derive; /**< Derive public key from private. */
    jolt_crypto_func_sign_t sign;     /**< Initialize multipart signing */
    jolt_crypto_func_verify_t verify; /**< Initialize multipart verifying */

} jolt_crypto_entry_t;

extern const jolt_crypto_entry_t jolt_crypto_registry[];

#endif
