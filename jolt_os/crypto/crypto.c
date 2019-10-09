#define LOG_LOCAL_LEVEL 4

#include "crypto_internal.h"
#include "string.h"
#include "esp_log.h"

static const char TAG[] = "jolt_crypto";

static jolt_crypto_status_t jolt_crypto_check_common( jolt_crypto_t *ctx )
{
    if( NULL == ctx ) return JOLT_CRYPTO_STATUS_FAIL;

    if( JOLT_CRYPTO_UNDEFINED == ctx->type ) {
        ESP_LOGE( TAG, "Must specify signing algorithm." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }
    else if( ctx->type >= JOLT_CRYPTO_MAX_IDX ) {
        ESP_LOGE( TAG, "Invalid signature algorithm identifier." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    if( JOLT_HASH_UNDEFINED == ctx->hash.type ) {
        ESP_LOGI(TAG, "Hashing algorith not set. Setting it to 0x%02X",
                jolt_crypto_registry[ctx->type].default_hash);
        ctx->hash.type = jolt_crypto_registry[ctx->type].default_hash;
    }

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_derive( jolt_crypto_t *ctx ) 
{ 
    jolt_crypto_status_t status;

    status = jolt_crypto_check_common( ctx );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    if( NULL == ctx->public || 0 == ctx->public_len ) {
        ESP_LOGE(TAG, "Must provide buffer for public key");
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    status = jolt_crypto_registry[ctx->type].derive(ctx);

    return status;
}

jolt_crypto_status_t jolt_crypto_sign( jolt_crypto_type_t sig_type, jolt_hash_type_t hash_type, uint8_t *sig,
                                       uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                       const uint8_t *private_key, uint16_t private_key_len )
{
    jolt_crypto_status_t status;
    jolt_crypto_t ctx = { 0 };

    ctx.type = sig_type;
    ctx.hash.type = hash_type;
    ctx.signature = sig;
    ctx.signature_len = sig_len;
    ctx.private = (uint8_t *) private_key;
    ctx.private_len = private_key_len;

    status = jolt_crypto_sign_init( &ctx );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_sign_update( &ctx, msg, msg_len );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_sign_final( &ctx );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_verify( jolt_crypto_type_t sig_type, jolt_hash_type_t hash_type, const uint8_t *sig,
                                         uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                         const uint8_t *public_key, uint16_t public_key_len )
{
    jolt_crypto_status_t status;
    jolt_crypto_t ctx = { 0 };

    ctx.type = sig_type;
    ctx.hash.type = hash_type;
    ctx.signature = (uint8_t *)sig;
    ctx.signature_len = sig_len;
    ctx.public = (uint8_t *)public_key;
    ctx.public_len = public_key_len;

    status = jolt_crypto_verify_init( &ctx );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_verify_update( &ctx, msg, msg_len );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_verify_final( &ctx );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_sign_init( jolt_crypto_t *ctx )
{ 
    jolt_crypto_status_t status;

    status = jolt_crypto_check_common( ctx );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_registry[ctx->type].sign_init( ctx );

    if( JOLT_CRYPTO_STATUS_SUCCESS == status ) { ctx->initialized = true; }

    return status;
}

jolt_crypto_status_t jolt_crypto_verify_init( jolt_crypto_t *ctx )
{ 
    jolt_crypto_status_t status;

    status = jolt_crypto_check_common( ctx );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_registry[ctx->type].verify_init( ctx );

    if( JOLT_CRYPTO_STATUS_SUCCESS == status ) { ctx->initialized = true; }

    return status;
}

jolt_crypto_status_t jolt_crypto_sign_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len )
{
    jolt_crypto_status_t status;

    status = jolt_crypto_check_common( ctx );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;
    if( false == ctx->initialized )
    {
        ESP_LOGE(TAG, "Crypto context not initialized");
        return JOLT_CRYPTO_STATUS_NOT_INIT;
    }

    if( NULL == msg || 0 == msg_len ) {
        ESP_LOGE( TAG, "No msg to sign." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    status = jolt_crypto_registry[ctx->type].sign_update( ctx, msg, msg_len );

    return status;
}

jolt_crypto_status_t jolt_crypto_verify_update( jolt_crypto_t *ctx, const uint8_t *msg, size_t msg_len )
{
    jolt_crypto_status_t status;

    status = jolt_crypto_check_common( ctx );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;
    if( false == ctx->initialized )
    {
        ESP_LOGE(TAG, "Crypto context not initialized");
        return JOLT_CRYPTO_STATUS_NOT_INIT;
    }

    if( NULL == msg || 0 == msg_len ) {
        ESP_LOGE( TAG, "No msg to sign." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    status = jolt_crypto_registry[ctx->type].verify_update( ctx, msg, msg_len );

    return status;
}


jolt_crypto_status_t jolt_crypto_sign_final( jolt_crypto_t *ctx ) 
{ 
    jolt_crypto_status_t status;

    status = jolt_crypto_check_common( ctx );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;
    if( false == ctx->initialized )
    {
        ESP_LOGE(TAG, "Crypto context not initialized");
        return JOLT_CRYPTO_STATUS_NOT_INIT;
    }

    status = jolt_crypto_registry[ctx->type].sign_final( ctx );
    return status;
}

jolt_crypto_status_t jolt_crypto_verify_final( jolt_crypto_t *ctx )
{ 
    jolt_crypto_status_t status;

    status = jolt_crypto_check_common( ctx );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;
    if( false == ctx->initialized )
    {
        ESP_LOGE(TAG, "Crypto context not initialized");
        return JOLT_CRYPTO_STATUS_NOT_INIT;
    }

    status = jolt_crypto_registry[ctx->type].verify_final( ctx );
    return status;
}

jolt_crypto_type_t jolt_crypto_from_str( const char *name )
{
    if( NULL == name ) return JOLT_CRYPTO_UNDEFINED;

    for( uint8_t i = 1; i < JOLT_CRYPTO_MAX_IDX; i++ ) {
        if( 0 == strcmp( jolt_crypto_registry[i].name, name ) ) return i;
    }

    return JOLT_CRYPTO_UNDEFINED;
}
