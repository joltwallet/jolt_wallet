#include "ed25519.h"
#include "sodium.h"
#include "string.h"

jolt_crypto_status_t jolt_crypto_ed25519_derive( uint8_t *public_key, uint16_t public_key_len,
        const uint8_t *private_key, uint16_t private_key_len )
{
    uint8_t seed[crypto_sign_ed25519_SEEDBYTES];

    if( 32 != private_key_len || 32 != public_key_len ) goto exit;

    if( crypto_sign_ed25519_sk_to_seed(seed, private_key) ) goto exit;

    if( crypto_sign_ed25519_seed_keypair(public_key, (uint8_t *) private_key, seed) ) goto exit;

    return JOLT_CRYPTO_STATUS_SUCCESS;

exit:
    sodium_memzero(seed, sizeof(seed));
    return JOLT_CRYPTO_STATUS_FAIL;
}

jolt_crypto_status_t jolt_crypto_ed25519_sign( uint8_t *sig, uint16_t sig_len,
                                                           const uint8_t *msg, size_t msg_len,
                                                           const uint8_t *public_key, uint16_t public_key_len,
                                                           const uint8_t *private_key, uint16_t private_key_len )
{
    int res;

    if( 32 != private_key_len || 32 != public_key_len || 64 != sig_len ) {
        return JOLT_CRYPTO_STATUS_PARAM; 
    }

    res = crypto_sign_ed25519_detached(sig, NULL, msg, msg_len, private_key);
    if( 0 != res ) return JOLT_CRYPTO_STATUS_FAIL;

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_ed25519_verify( const uint8_t *sig, uint16_t sig_len,
        const uint8_t *msg, size_t msg_len,
        const uint8_t *public_key, uint16_t public_key_len )
{
    int res;

    if( 32 != public_key_len || 64 != sig_len ) {
        return JOLT_CRYPTO_STATUS_PARAM; 
    }

    res = crypto_sign_ed25519_verify_detached(sig, msg, msg_len, public_key);
    if( 0 != res ) return JOLT_CRYPTO_STATUS_FAIL;

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

