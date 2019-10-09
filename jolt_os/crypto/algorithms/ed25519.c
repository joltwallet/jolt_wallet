#define LOG_LOCAL_LEVEL 4

#include "ed25519.h"
#include "sodium/private/curve25519_ref10.h"
#include <string.h>
#include <byteswap.h>
#include "sodium.h"
#include "esp_log.h"

static const char TAG[] = "jolt_crypto/ed25519";

typedef struct {
    uint8_t az[64];
} jolt_crypto_ed25519_state_t;

jolt_crypto_status_t jolt_crypto_ed25519_derive( jolt_crypto_t *ctx ) 
{ 
    jolt_crypto_status_t status;
    uint8_t hash[64];
    
    ctx->hash.hash = hash;
    ctx->hash.hash_len = sizeof(hash);

    if( ctx->private_len != 32 ) {
        ESP_LOGE(TAG, "Private key must be 32 long.");
        return JOLT_CRYPTO_STATUS_PARAM;
    }
    else if( ctx->public_len != 32 ) {
        ESP_LOGE(TAG, "Public key must be 32 long.");
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    status = jolt_hash_init( &ctx->hash );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_update( &ctx->hash, ctx->private, ctx->private_len); 
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_final( &ctx->hash );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    hash[0] &= 248;
    hash[31] &= 63;
    hash[31] |= 64;
  
    {
        ge_p3 A;
        ge_scalarmult_base(&A, hash);
        ge_p3_tobytes(ctx->public, &A);
    }

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_ed25519_sign_init( jolt_crypto_t *ctx )
{ 
    // TODO: Always derive public key?
    jolt_crypto_status_t status;
    jolt_crypto_ed25519_state_t *state;

    state = calloc(1, sizeof(jolt_crypto_ed25519_state_t));
    if( NULL == state ) return JOLT_CRYPTO_STATUS_OOM;
    ctx->state = state;

    ctx->hash.hash = state->az;
    ctx->hash.hash_len = 64;

    status = jolt_hash_init( &ctx->hash );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_update( &ctx->hash, ctx->private, ctx->private_len);
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_final( &ctx->hash);
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    state->az[0] &= 248;
    state->az[31] &= 63;
    state->az[31] |= 64;


    status = jolt_hash_init( &ctx->hash );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_update( &ctx->hash, state->az + 32, 32);
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_ed25519_verify_init( jolt_crypto_t *ctx )
{
    jolt_crypto_status_t status;
    jolt_crypto_ed25519_state_t *state = ctx->state;

    if (ctx->signature[63] & 224) return JOLT_CRYPTO_STATUS_INVALID_SIG;

    {
        unsigned char            d = 0;
        for (uint8_t i = 0; i < 32; ++i) {
            d |= ctx->public[i];
        }
        if (d == 0) {
            return JOLT_CRYPTO_STATUS_INVALID_PUB;
        }
    }

    // Generate 64 bytes (512 bits) from private key into az
    ctx->hash.hash = state->az;
    ctx->hash.hash_len = 64;

    status = jolt_hash_init( &ctx->hash );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_update( &ctx->hash, ctx->signature, 32);
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_update( &ctx->hash, ctx->public, 32);
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_ed25519_sign_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len )
{
    return jolt_hash_update( &ctx->hash, msg, msg_len);
}

jolt_crypto_status_t jolt_crypto_ed25519_verify_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len )
{
    return jolt_hash_update( &ctx->hash, msg, msg_len);
}

jolt_crypto_status_t jolt_crypto_ed25519_verify_final( jolt_crypto_t *ctx ) 
{ 
    jolt_crypto_status_t status;
    jolt_crypto_ed25519_state_t *state = ctx->state;
    ge_p3                    A;
    ge_p2                    R;
    unsigned char            rcheck[32];

    status = jolt_hash_final( &ctx->hash );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    if (ge_frombytes_negate_vartime(&A, ctx->public) != 0) {
        return JOLT_CRYPTO_STATUS_INVALID_SIG;
    }

    sc_reduce(state->az);
    ge_double_scalarmult_vartime(&R, state->az, &A, ctx->signature + 32);
    ge_tobytes(rcheck, &R);

    if( 0 != (crypto_verify_32(rcheck, ctx->signature) 
            | (-(rcheck == ctx->signature)) 
            | sodium_memcmp(ctx->signature, rcheck, 32))
            ) {
        return JOLT_CRYPTO_STATUS_INVALID_SIG;
    }

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_ed25519_sign_final( jolt_crypto_t *ctx ) 
{ 
    jolt_crypto_status_t status;
    jolt_crypto_ed25519_state_t *state = ctx->state;
    unsigned char hram[64];
    unsigned char nonce[64];
    ge_p3 R;

    ctx->hash.hash = nonce;
    status = jolt_hash_final(&ctx->hash);

    if( 64 != ctx->signature_len ) return JOLT_CRYPTO_STATUS_PARAM;

    memcpy(ctx->signature + 32, ctx->public, 32); // set upper 32 bits of sig

    sc_reduce(nonce);
    ge_scalarmult_base(&R, nonce);
    ge_p3_tobytes(ctx->signature, &R);

    ctx->hash.hash = hram;

    status = jolt_hash_init( &ctx->hash );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_update( &ctx->hash, ctx->signature, ctx->signature_len );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_final(&ctx->hash);
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    sc_reduce(hram);
    sc_muladd(ctx->signature + 32, hram, state->az, nonce);

    sodium_memzero(state, sizeof(jolt_crypto_ed25519_state_t));
    sodium_memzero(hram, sizeof(hram));

    free(ctx->state);
    ctx->state = NULL;

    return JOLT_CRYPTO_STATUS_SUCCESS;
}
