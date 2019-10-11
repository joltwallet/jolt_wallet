#define LOG_LOCAL_LEVEL 4

#include "crypto_internal.h"
#include "string.h"
#include "esp_log.h"

static const char TAG[] = "jolt_crypto";

static jolt_crypto_status_t jolt_crypto_check_type( jolt_crypto_type_t type )
{
    if( JOLT_CRYPTO_UNDEFINED == type ) {
        ESP_LOGE( TAG, "Must specify signing algorithm." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }
    else if( type >= JOLT_CRYPTO_MAX_IDX ) {
        ESP_LOGE( TAG, "Invalid signature algorithm identifier." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

static jolt_crypto_status_t jolt_crypto_check_nonzero( const uint8_t *key, uint16_t len )
{
    if( NULL == key || 0 == len ) {
        ESP_LOGE(TAG, "Must provide key");
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    return JOLT_CRYPTO_STATUS_SUCCESS;
}

jolt_crypto_status_t jolt_crypto_derive( jolt_crypto_type_t type,
        uint8_t *public_key, uint16_t public_key_len,
        const uint8_t *private_key, uint16_t private_key_len )
{ 
    jolt_crypto_status_t status;

    status = jolt_crypto_check_type( type );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( public_key, public_key_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( private_key, private_key_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_registry[type].derive(
            public_key, public_key_len,
            private_key, private_key_len);

    return status;
}

jolt_crypto_status_t jolt_crypto_sign( jolt_crypto_type_t type, uint8_t *sig,
                                       uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                       const uint8_t *public_key, uint16_t public_key_len,
                                       const uint8_t *private_key, uint16_t private_key_len )
{
    jolt_crypto_status_t status;

    status = jolt_crypto_check_type( type );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( sig, sig_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( msg, msg_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( public_key, public_key_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( private_key, private_key_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    // TODO: Make macro to verify public key

    status = jolt_crypto_registry[type].sign(
            sig, sig_len,
            msg, msg_len,
            public_key, public_key_len,
            private_key, private_key_len);

    return status;
}

jolt_crypto_status_t jolt_crypto_verify( jolt_crypto_type_t type, const uint8_t *sig,
                                         uint16_t sig_len, const uint8_t *msg, size_t msg_len,
                                         const uint8_t *public_key, uint16_t public_key_len )
{
    jolt_crypto_status_t status;

    status = jolt_crypto_check_type( type );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( sig, sig_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( msg, msg_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_check_nonzero( public_key, public_key_len );
    if (JOLT_CRYPTO_STATUS_SUCCESS != status ) return status;

    status = jolt_crypto_registry[type].verify(
            sig, sig_len,
            msg, msg_len,
            public_key, public_key_len);

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
