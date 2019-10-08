#define LOG_LOCAL_LEVEL 4

#include "assert.h"
#include "esp_log.h"
#include "hash_internal.h"

jolt_hash_status_t jolt_hash_sha256( uint8_t *hash, size_t hash_len, const uint8_t *msg, size_t msg_len,
                                     const uint8_t *key, uint8_t key_len )
{
    int res;
    assert( hash_len == 32 );
    assert( key_len == 0 );
    res = crypto_hash_sha256( hash, msg, msg_len );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}

jolt_hash_status_t jolt_hash_sha256_init( jolt_hash_t *ctx )
{
    int res;
    crypto_hash_sha256_state *state;
    state = calloc( 1, sizeof( crypto_hash_sha256_state ) );
    if( NULL == state ) return JOLT_HASH_STATUS_OOM;
    res = crypto_hash_sha256_init( state );

    if( 0 != res ) {
        free( state );
        return JOLT_HASH_STATUS_FAIL;
    }
    ctx->state = state;
    return JOLT_HASH_STATUS_SUCCESS;
}

jolt_hash_status_t jolt_hash_sha256_update( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len )
{
    if( NULL == ctx->state ) return JOLT_HASH_STATUS_FAIL;

    int res;
    res = crypto_hash_sha256_update( ctx->state, msg, msg_len );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}

jolt_hash_status_t jolt_hash_sha256_final( jolt_hash_t *ctx )
{
    if( NULL == ctx->state ) return JOLT_HASH_STATUS_FAIL;

    int res;
    res = crypto_hash_sha256_final( ctx->state, ctx->hash );
    free( ctx->state );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}
