#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "hash_internal.h"

static const char TAG[] = "jolt_hash";

/**
 * @brief Common validity checks on the jolt hashing context
 * @param[in] Hashing context.
 * @return Returns JOLT_HASH_STATUS_SUCCESS on passing common validity checks.
 */
static jolt_hash_status_t check_hashing_ctx_common( jolt_hash_t *ctx )
{
    if( NULL == ctx ) return JOLT_HASH_STATUS_PARAM;
    if( JOLT_HASH_UNDEFINED == ctx->type ) {
        ESP_LOGE( TAG, "Must specify hashing algorithm." );
        return JOLT_HASH_STATUS_PARAM;
    }
    else if( ctx->type >= JOLT_HASH_MAX_IDX ) {
        ESP_LOGE( TAG, "Invalid hashing identifier." );
        return JOLT_HASH_STATUS_PARAM;
    }

    if( NULL == ctx->hash || 0 == ctx->hash_len ) {
        ESP_LOGE( TAG, "Invalid output hash buffer" );
        return JOLT_HASH_STATUS_PARAM;
    }

    return JOLT_HASH_STATUS_SUCCESS;
}

jolt_hash_status_t jolt_hash( jolt_hash_type_t type, uint8_t *hash, size_t hash_len, const uint8_t *msg,
                              size_t msg_len, const uint8_t *key, uint8_t key_len )
{
    if( JOLT_HASH_UNDEFINED == type ) {
        ESP_LOGE( TAG, "Must specify hashing algorithm." );
        return JOLT_HASH_STATUS_PARAM;
    }
    else if( type >= JOLT_HASH_MAX_IDX ) {
        ESP_LOGE( TAG, "Invalid hashing identifier." );
        return JOLT_HASH_STATUS_PARAM;
    }

    if( NULL == hash || 0 == hash_len ) {
        ESP_LOGE( TAG, "Must provide output hashing buffer." );
        return JOLT_HASH_STATUS_PARAM;
    }

    if( NULL == msg || 0 == msg_len ) {
        ESP_LOGE( TAG, "No msg to hash." );
        return JOLT_HASH_STATUS_PARAM;
    }

    if( NULL == key && 0 != key_len ) {
        ESP_LOGE( TAG, "NULL key provided, but key_len is %d", key_len );
        return JOLT_HASH_STATUS_PARAM;
    }

    return jolt_hash_registry[type].single( hash, hash_len, msg, msg_len, key, key_len );
}

jolt_hash_status_t jolt_hash_init( jolt_hash_t *ctx )
{
    jolt_hash_status_t status;

    status = check_hashing_ctx_common( ctx );
    if( JOLT_HASH_STATUS_SUCCESS != status ) return status;

    status = jolt_hash_registry[ctx->type].init( ctx );

    if( JOLT_HASH_STATUS_SUCCESS == status ) { ctx->initialized = true; }

    return status;
}

jolt_hash_status_t jolt_hash_update( jolt_hash_t *ctx, const uint8_t *msg, size_t msg_len )
{
    jolt_hash_status_t status;

    status = check_hashing_ctx_common( ctx );
    if( JOLT_HASH_STATUS_SUCCESS != status ) return status;

    if( false == ctx->initialized ) {
        ESP_LOGE( TAG, "Hashing context not initialized" );
        return JOLT_HASH_STATUS_NOT_INIT;
    }

    if( NULL == msg || 0 == msg_len ) {
        ESP_LOGE( TAG, "No msg to hash." );
        return JOLT_HASH_STATUS_PARAM;
    }

    status = jolt_hash_registry[ctx->type].update( ctx, msg, msg_len );

    return status;
}

jolt_hash_status_t jolt_hash_final( jolt_hash_t *ctx )
{
    jolt_hash_status_t status;

    status = check_hashing_ctx_common( ctx );
    if( JOLT_HASH_STATUS_SUCCESS != status ) return status;

    if( false == ctx->initialized ) {
        ESP_LOGE( TAG, "Hashing context not initialized" );
        return JOLT_HASH_STATUS_NOT_INIT;
    }

    status = jolt_hash_registry[ctx->type].final( ctx );

    return status;
}
