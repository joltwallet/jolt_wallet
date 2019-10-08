#include "crypto_internal.h"
#include "string.h"

jolt_crypto_status_t jolt_crypto_derive( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_crypto_status_t jolt_crypto_set_private( jolt_crypto_t *ctx, const uint8_t *key, uint16_t key_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_set_public( jolt_crypto_t *ctx, const uint8_t *key, uint16_t key_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_free( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_crypto_status_t jolt_crypto_sign( jolt_crypto_type_t sig_type, jolt_hash_type_t hash_type, uint8_t *sig,
                                       uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                       const uint8_t *private_key, uint16_t private_key_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_verify( jolt_crypto_type_t sig_type, jolt_hash_type_t hash_type, const uint8_t *sig,
                                         uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                         const uint8_t *public_key, uint16_t public_key_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_init( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_crypto_status_t jolt_crypto_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len )
{
    return JOLT_CRYPTO_STATUS_NOT_IMPL;
}

jolt_crypto_status_t jolt_crypto_verify_final( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_crypto_status_t jolt_crypto_sign_final( jolt_crypto_t *ctx ) { return JOLT_CRYPTO_STATUS_NOT_IMPL; }

jolt_hash_type_t jolt_crypto_from_str( const char *name )
{
    if( NULL == name ) return JOLT_CRYPTO_UNDEFINED;

    for( uint8_t i = 1; i < JOLT_CRYPTO_MAX_IDX; i++ ) {
        if( 0 == strcmp( jolt_crypto_registry[i].name, name ) ) return i;
    }

    return JOLT_CRYPTO_UNDEFINED;
}
