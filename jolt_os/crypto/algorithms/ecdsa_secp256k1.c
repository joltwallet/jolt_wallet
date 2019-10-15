//#define LOG_LOCAL_LEVEL 4

#include "ecdsa_secp256k1.h"
#include <mbedtls/ecdsa.h>
#include "assert.h"
#include "esp_log.h"
#include "jolt_helpers.h"
#include "string.h"

static const char TAG[] = "ecdsa_secp256k1";

/**
 * @brief Populate a keypair object.
 *
 * Populates group, private key, and public key.
 *
 * @param[out] keypair Populated keypair object. Must be previously initialized.
 */
jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_keypair( mbedtls_ecp_keypair *keypair, const uint8_t *private_key )
{
    int res;
    jolt_crypto_status_t status = JOLT_CRYPTO_STATUS_FAIL;

    res = mbedtls_mpi_read_binary( &keypair->d, private_key, 32 );

    /* Populate secret */
    res = mbedtls_mpi_read_binary( &keypair->d, private_key, 32 );
    if( 0 != res ) goto exit;

    res = mbedtls_ecp_group_load( &keypair->grp, MBEDTLS_ECP_DP_SECP256K1 );
    if( 0 != res ) goto exit;

    /* check valid private key range */
    {
        mbedtls_mpi upper;

        mbedtls_mpi_init( &upper );
        assert( 0 == mbedtls_mpi_read_string( &upper, 16,
                                              "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364140" ) );

        if( 1 == mbedtls_mpi_cmp_mpi( &keypair->d, &upper ) || 1 != mbedtls_mpi_cmp_int( &keypair->d, 0 ) ) {
            /* Private Key wasn't in valid range */
            status = JOLT_CRYPTO_STATUS_INVALID_PRIV;
            goto exit;
        }
    }

    /* Derive Public Key */
    res = mbedtls_ecp_mul( &keypair->grp, &keypair->Q, &keypair->d, &keypair->grp.G, NULL, NULL );
    if( 0 != res ) goto exit;

    if( LOG_LOCAL_LEVEL >= 4 ) {
        char buf[256];
        size_t olen;

        mbedtls_mpi_write_string( &keypair->d, 16, buf, sizeof( buf ), &olen );
        ESP_LOGD( TAG, "d: %s", buf );

        mbedtls_mpi_write_string( &keypair->Q.X, 16, buf, sizeof( buf ), &olen );
        ESP_LOGD( TAG, "Q.X: %s", buf );

        mbedtls_mpi_write_string( &keypair->Q.Y, 16, buf, sizeof( buf ), &olen );
        ESP_LOGD( TAG, "Q.Y: %s", buf );

        mbedtls_mpi_write_string( &keypair->Q.Z, 16, buf, sizeof( buf ), &olen );
        ESP_LOGD( TAG, "Q.Z: %s", buf );
    }

    status = JOLT_CRYPTO_STATUS_SUCCESS;

exit:

    return status;
}

jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_derive( uint8_t *public_key, uint16_t *public_key_len,
                                                         const uint8_t *private_key, uint16_t private_key_len )
{
    int res;
    jolt_crypto_status_t status = JOLT_CRYPTO_STATUS_FAIL;
    mbedtls_ecp_keypair keypair;
    mbedtls_ecp_keypair_init( &keypair );

    /* Confirm input/output buffer sizes */
    if( 65 != *public_key_len || 32 != private_key_len ) {
        status = JOLT_CRYPTO_STATUS_PARAM;
        goto exit;
    }

    status = jolt_crypto_ecdsa_secp256k1_keypair( &keypair, private_key );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) goto exit;
    status = JOLT_CRYPTO_STATUS_FAIL;

    /* Export Q to uncompressed public key {0x04, X[32], Y[32]}*/
    {
        size_t olen;
        res = mbedtls_ecp_point_write_binary( &keypair.grp, &keypair.Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &olen, public_key,
                                              *public_key_len );
        if( 0 != res ) goto exit;
    }

    status = JOLT_CRYPTO_STATUS_SUCCESS;

exit:
    mbedtls_ecp_keypair_free( &keypair );

    return status;
}

jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_sign( uint8_t *sig, uint16_t *sig_len, const uint8_t *msg,
                                                       size_t msg_len, const uint8_t *public_key,
                                                       uint16_t public_key_len, const uint8_t *private_key,
                                                       uint16_t private_key_len )
{
    int res;
    jolt_crypto_status_t status = JOLT_CRYPTO_STATUS_FAIL;
    mbedtls_ecdsa_context ctx;
    mbedtls_ecp_keypair keypair;

    mbedtls_ecdsa_init( &ctx );
    mbedtls_ecp_keypair_init( &keypair );

    /* Confirm input parameters */
    if( *sig_len < ( 2 * 32 + 9 ) ) {
        /* Must support maximum signature length size */
        ESP_LOGE( TAG, "Insufficient signature buffer." );
        return JOLT_CRYPTO_STATUS_INSUFF_BUF;
    }
    else if( 32 != msg_len ) {
        /* This is for signing a message hash, not the message itself */
        ESP_LOGE( TAG, "msg should be a 32-byte hash." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    if( LOG_LOCAL_LEVEL >= 4 ) {
        ESP_LOGD( TAG, "Signing hash: " );
        jolt_print_bytearray( msg, msg_len, false );
    }

    /* Populate the keypair */
    status = jolt_crypto_ecdsa_secp256k1_keypair( &keypair, private_key );
    if( JOLT_CRYPTO_STATUS_SUCCESS != status ) goto exit;
    status = JOLT_CRYPTO_STATUS_FAIL;

    // TODO: compare derived public key with provided one for safety.

    /* Instantiate the signing context */
    res = mbedtls_ecdsa_from_keypair( &ctx, &keypair );
    if( 0 != res ) goto exit;

    /* Sign */
    size_t slen;
    res = mbedtls_ecdsa_write_signature( &ctx, MBEDTLS_MD_SHA256, msg, msg_len, sig, &slen, NULL, NULL );
    if( 0 != res ) goto exit;
    *sig_len = slen;

    if( LOG_LOCAL_LEVEL >= 4 ) {
        ESP_LOGD( TAG, "Produced signature (%d):", slen );
        jolt_print_bytearray( sig, slen, false );
    }

    status = JOLT_CRYPTO_STATUS_SUCCESS;

exit:
    mbedtls_ecdsa_free( &ctx );
    mbedtls_ecp_keypair_free( &keypair );

    return status;
}

jolt_crypto_status_t jolt_crypto_ecdsa_secp256k1_verify( const uint8_t *sig, uint16_t sig_len, const uint8_t *msg,
                                                         size_t msg_len, const uint8_t *public_key,
                                                         uint16_t public_key_len )
{
    /* Requires DER encoded sig */
    int res;
    jolt_crypto_status_t status = JOLT_CRYPTO_STATUS_FAIL;
    mbedtls_ecdsa_context ctx;
    mbedtls_ecp_keypair keypair;

    mbedtls_ecdsa_init( &ctx );
    mbedtls_ecp_keypair_init( &keypair );

    if( 32 != msg_len ) {
        /* This is for signing a message hash, not the message itself */
        ESP_LOGE( TAG, "msg should be a 32-byte hash." );
        return JOLT_CRYPTO_STATUS_PARAM;
    }

    res = mbedtls_ecp_group_load( &keypair.grp, MBEDTLS_ECP_DP_SECP256K1 );
    if( 0 != res ) goto exit;

    res = mbedtls_ecp_point_read_binary( &keypair.grp, &keypair.Q, public_key, public_key_len );
    if( 0 != res ) goto exit;

    res = mbedtls_ecdsa_from_keypair( &ctx, &keypair );
    if( 0 != res ) goto exit;

    if( LOG_LOCAL_LEVEL >= 4 ) {
        char buf[256];
        size_t olen;

        ESP_LOGD( TAG, "Verifying hash: " );
        jolt_print_bytearray( msg, msg_len, false );

        mbedtls_mpi_write_string( &keypair.Q.X, 16, buf, sizeof( buf ), &olen );
        ESP_LOGD( TAG, "Q.X: %s", buf );

        mbedtls_mpi_write_string( &keypair.Q.Y, 16, buf, sizeof( buf ), &olen );
        ESP_LOGD( TAG, "Q.Y: %s", buf );
    }

    res = mbedtls_ecdsa_read_signature( &ctx, msg, msg_len, sig, sig_len );
    if( MBEDTLS_ERR_ECP_BAD_INPUT_DATA == res ) {
        ESP_LOGE( TAG, "Invalid signature" );
        if( LOG_LOCAL_LEVEL >= 4 ) {
            ESP_LOGD( TAG, "Signature len %d", sig_len );
            jolt_print_bytearray( sig, sig_len, false );
        }
        goto exit;
    }
    else if( 0 != res ) {
        ESP_LOGE( TAG, "Failed to read/verify signature (-0x%04X)", (uint16_t)-res );
        goto exit;
    }

    status = JOLT_CRYPTO_STATUS_SUCCESS;

exit:
    mbedtls_ecdsa_free( &ctx );
    mbedtls_ecp_keypair_free( &keypair );

    return status;
}
