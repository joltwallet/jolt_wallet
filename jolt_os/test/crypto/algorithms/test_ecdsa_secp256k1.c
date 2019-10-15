//#define LOG_LOCAL_LEVEL 4

#include "algorithms/ecdsa_secp256k1.h"
#include "esp_log.h"
#include "jolt_crypto.h"
#include "jolt_hash.h"
#include "jolt_helpers.h"
#include "sodium.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_crypto/ecdsa_secp256k1]";

TEST_CASE( "ecdsa secp256k1 basic", MODULE_NAME )
{
    jolt_crypto_status_t status;
    uint8_t private[32], public[65], signature[73];
    uint16_t public_len    = sizeof( public );
    uint16_t signature_len = sizeof( signature );
    uint16_t bad_public_len;
    const char msg[] = "jolt wallet";
    uint8_t hash[32];

    /* Test invalid params */
    bad_public_len = 64;
    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &bad_public_len, private, sizeof( private ) );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_PARAM, status );

    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &public_len, private, 31 );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_PARAM, status );

    {
        /* Populate Private Key */
        const char hex[] = "1294d9f787ad6274b3814badd3871966ed6c11cd684feff1515f4014827a2d9e";
        int res;
        res = sodium_hex2bin( private, sizeof( private ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Test Public Key Derivation */
    status = jolt_crypto_derive( JOLT_CRYPTO_ECDSA_SECP256K1, public, &public_len, private, sizeof( private ) );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_SUCCESS, status );

    {
        uint8_t expected_public[65];
        const char hex[] = "044bf1a09fd56e331f48690b76916eb21bd136f4b1de3b27b5c3d2c07d570fc746"
                           "8e031954e4273d603e7f7b8de2035b4cdfb39700373a61bc28d5441c5d482b50";
        int res;
        res = sodium_hex2bin( expected_public, sizeof( expected_public ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_public, public, sizeof( expected_public ) );
    }

    /* Test Computed Signature */
    // First, hash the message
    {
        jolt_hash_status_t status;
        status = jolt_hash( JOLT_HASH_SHA256, hash, sizeof( hash ), (uint8_t*)msg, strlen( msg ), NULL, 0 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );

        uint8_t expected_hash[32];
        const char hex[] = "594fca9d5dfec1a1d95e89f79e08144723a581032f2434553f168218dc2af49b";
        int res;
        res = sodium_hex2bin( expected_hash, sizeof( expected_hash ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_hash, hash, 32 );
    }

    status = jolt_crypto_sign( JOLT_CRYPTO_ECDSA_SECP256K1, signature, &signature_len, hash, sizeof( hash ), public,
                               sizeof( public ), private, sizeof( private ) );
    TEST_ASSERT_JOLT_CRYPTO_STATUS( JOLT_CRYPTO_STATUS_SUCCESS, status );

    {
        /* DER encoded */
        uint8_t expected_signature[71];
        const char hex[] = "30450220576d8830d97ebddc809c12d1940824c615319f1861617c80aeb062af2c1237f"
                           "e02210081455f9556a4cc29eb70cbc2c3f1eb221bcd6ad2bd8849858d91bac71a3b42c8";
        int res;
        res = sodium_hex2bin( expected_signature, sizeof( expected_signature ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );

        TEST_ASSERT_EQUAL_INT( sizeof( expected_signature ), signature_len );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_signature, signature, sizeof( expected_signature ) );

        /* Verify Signature */
#if 1
        status = jolt_crypto_verify( JOLT_CRYPTO_ECDSA_SECP256K1, signature, signature_len, hash, sizeof( hash ),
                                     public, sizeof( public ) );
#else
        status = jolt_crypto_verify( JOLT_CRYPTO_ECDSA_SECP256K1, expected_signature, sizeof( expected_signature ),
                                     hash, sizeof( hash ), public, sizeof( public ) );
#endif
        TEST_ASSERT_EQUAL_HEX8( JOLT_CRYPTO_STATUS_SUCCESS, status );
    }
}

TEST_CASE( "mbedtls direct", MODULE_NAME )
{
    int ret = 1;
    mbedtls_ecdsa_context ctx_sign, ctx_verify;
    unsigned char hash[32];
    unsigned char sig[MBEDTLS_ECDSA_MAX_LEN];
    size_t sig_len;

    mbedtls_ecdsa_init( &ctx_sign );
    mbedtls_ecdsa_init( &ctx_verify );

    {
        /* Populate Private Key */
        uint8_t private[32];
        const char hex[] = "1294d9f787ad6274b3814badd3871966ed6c11cd684feff1515f4014827a2d9e";
        mbedtls_ecp_keypair keypair;
        ret = sodium_hex2bin( private, sizeof( private ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, ret );
        mbedtls_ecp_keypair_init( &keypair );
        ret = jolt_crypto_ecdsa_secp256k1_keypair( &keypair, private );
        TEST_ASSERT_EQUAL_INT( 0, ret );
        ret = mbedtls_ecdsa_from_keypair( &ctx_sign, &keypair );
        TEST_ASSERT_EQUAL_INT( 0, ret );
    }

    /*
     * Compute message hash
     */
    {
        const char hex[] = "594fca9d5dfec1a1d95e89f79e08144723a581032f2434553f168218dc2af49b";
        sodium_hex2bin( hash, sizeof( hash ), hex, strlen( hex ), NULL, NULL, NULL );
    }

    /*
     * Sign message hash
     */
    ret = mbedtls_ecdsa_write_signature( &ctx_sign, MBEDTLS_MD_SHA256, hash, sizeof( hash ), sig, &sig_len, NULL,
                                         NULL );
    TEST_ASSERT_EQUAL_INT( 0, ret );

    /*
     * Transfer public information to verifying context
     *
     * We could use the same context for verification and signatures, but we
     * chose to use a new one in order to make it clear that the verifying
     * context only needs the public key (Q), and not the private key (d).
     */
    ret = mbedtls_ecp_group_load( &ctx_verify.grp, MBEDTLS_ECP_DP_SECP256K1 );
    TEST_ASSERT_EQUAL_INT( 0, ret );

    ret = mbedtls_ecp_copy( &ctx_verify.Q, &ctx_sign.Q );
    TEST_ASSERT_EQUAL_INT( 0, ret );

    if( LOG_LOCAL_LEVEL >= 4 ) {
        ESP_LOGD( MODULE_NAME, "Signature (%d):", sig_len );
        jolt_print_bytearray( sig, sig_len, false );
    }

    /*
     * Verify signature
     */
    ret = mbedtls_ecdsa_read_signature( &ctx_verify, hash, sizeof( hash ), sig, sig_len );
    TEST_ASSERT_EQUAL_INT( 0, ret );
}
