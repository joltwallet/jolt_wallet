#define LOG_LOCAL_LEVEL 4

#include "assert.h"
#include "esp_log.h"
#include "hash_internal.h"
#include "mbedtls/ripemd160.h"

jolt_hash_status_t jolt_hash_ripemd160( uint8_t *hash, size_t hash_len, const uint8_t *msg, size_t msg_len,
                                     const uint8_t *key, uint8_t key_len )
{
    int res;
    assert( hash_len == 20 );
    assert( key_len == 0 );
    res = mbedtls_ripemd160_ret( msg, msg_len, hash);
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}

jolt_hash_status_t jolt_hash_ripemd160_init( jolt_hash_t *ctx )
{
    int res;
    mbedtls_ripemd160_context *state;
    state = calloc( 1, sizeof( mbedtls_ripemd160_context ) );
    if( NULL == state ) return JOLT_HASH_STATUS_OOM;
    mbedtls_ripemd160_init( state );
    res = mbedtls_ripemd160_starts_ret( state );
    if( 0 != res ) {
        free(state);
        return JOLT_HASH_STATUS_FAIL;
    }
    ctx->state = state;
    return JOLT_HASH_STATUS_SUCCESS;
}

jolt_hash_status_t jolt_hash_ripemd160_update( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len )
{
    if( NULL == ctx->state ) return JOLT_HASH_STATUS_FAIL;

    int res;
    res = mbedtls_ripemd160_update_ret( ctx->state, msg, msg_len );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}

jolt_hash_status_t jolt_hash_ripemd160_final( jolt_hash_t *ctx )
{
    if( NULL == ctx->state ) return JOLT_HASH_STATUS_FAIL;

    int res;
    res = mbedtls_ripemd160_finish_ret( ctx->state, ctx->hash );
    free( ctx->state );
    if( 0 == res )
        return JOLT_HASH_STATUS_SUCCESS;
    else
        return JOLT_HASH_STATUS_FAIL;
}
