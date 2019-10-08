#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "hash_internal.h"

jolt_hash_status_t jolt_hash_blake2b( uint8_t *hash, size_t hash_len, const uint8_t *msg, size_t msg_len,
                                      const uint8_t *key, uint8_t key_len )
{
    int res;
    res = crypto_generichash_blake2b( hash, hash_len, msg, msg_len, key, key_len );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}

jolt_hash_status_t jolt_hash_blake2b_init( jolt_hash_t *ctx )
{
    int res;
    crypto_generichash_blake2b_state *state;
    state = calloc( 1, sizeof( crypto_generichash_blake2b_state ) );
    if( NULL == state ) return JOLT_HASH_STATUS_OOM;
    res = crypto_generichash_blake2b_init( state, ctx->key, ctx->key_len, ctx->hash_len );

    if( 0 != res ) {
        free( state );
        return JOLT_HASH_STATUS_FAIL;
    }
    ctx->state = state;
    return JOLT_HASH_STATUS_SUCCESS;
}

jolt_hash_status_t jolt_hash_blake2b_update( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len )
{
    if( NULL == ctx->state ) return JOLT_HASH_STATUS_FAIL;

    int res;
    res = crypto_generichash_blake2b_update( ctx->state, msg, msg_len );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}

jolt_hash_status_t jolt_hash_blake2b_final( jolt_hash_t *ctx )
{
    if( NULL == ctx->state ) return JOLT_HASH_STATUS_FAIL;

    int res;
    res = crypto_generichash_blake2b_final( ctx->state, ctx->hash, ctx->hash_len );
    free( ctx->state );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}
