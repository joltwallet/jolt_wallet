#ifndef JOLT_CRYPTO_ALGORITHMS_ED25519_H__
#define JOLT_CRYPTO_ALGORITHMS_ED25519_H__

#include "jolt_crypto.h"

jolt_crypto_status_t jolt_crypto_ed25519_derive( jolt_crypto_t *ctx );

jolt_crypto_status_t jolt_crypto_ed25519_sign_init( jolt_crypto_t *ctx );
jolt_crypto_status_t jolt_crypto_ed25519_verify_init( jolt_crypto_t *ctx );

jolt_crypto_status_t jolt_crypto_ed25519_sign_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len );
jolt_crypto_status_t jolt_crypto_ed25519_verify_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len );

jolt_crypto_status_t jolt_crypto_ed25519_sign_final( jolt_crypto_t *ctx );
jolt_crypto_status_t jolt_crypto_ed25519_verify_final( jolt_crypto_t *ctx );

#endif
