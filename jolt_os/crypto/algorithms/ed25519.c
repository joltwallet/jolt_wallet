#include "ed25519.h"

jolt_crypto_status_t jolt_crypto_ed25519_derive( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_crypto_status_t jolt_crypto_ed25519_sign( jolt_hash_type_t hash_type, uint8_t *sig, uint16_t sig_len,
                                               const uint8_t *msg, size_t msg_len, const uint8_t *private_key,
                                               uint16_t private_key_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_ed25519_verify( jolt_hash_type_t hash_type, const uint8_t *sig, uint16_t sig_len,
                                                 const uint8_t *msg, size_t msg_len, const uint8_t *public_key,
                                                 uint16_t public_key_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_ed25519_init( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_crypto_status_t jolt_crypto_ed25519_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_ed25519_verify_final( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_crypto_status_t jolt_crypto_ed25519_sign_final( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }
