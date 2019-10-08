#include "jolt_hash.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_hash]";

TEST_CASE( "jolt_hash single", MODULE_NAME )
{
    jolt_hash_status_t status;
    uint8_t expected[64], out[64];
    const uint8_t msg[] = {0x00, 0x01, 0x02, 0x03};
    uint8_t key[64];

    /*************************
     * Populate Test-Vectors *
     *************************/
    {
        /* Populate Key */
        const char hex[] = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c"
                           "2d2e2f303132333435363738393a3b3c3d3e3f";
        int res;

        res = sodium_hex2bin( key, sizeof( key ), hex, sizeof( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    {
        /* Populate Key */
        const char hex[] = "32f398a60c1e53f1f81d6d8da2ec1175422d6b2cfa0c0e66d8c4e730b296a4b53e392e39859822a145ae5f1a24"
                           "c27f55339e2b4b4458e8c5eb19aa14206427aa";
        int res;

        res = sodium_hex2bin( expected, sizeof( expected ), hex, sizeof( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /****************************
     * Check for invalid params *
     ****************************/

    /* Should reject JOLT_HASH_UNDEFINED */
    status = jolt_hash( JOLT_HASH_UNDEFINED, out, sizeof( out ), msg, sizeof( msg ), key, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_PARAM, status );

    /* Should reject JOLT_HASH_IDX_MAX */
    status = jolt_hash( JOLT_HASH_MAX_IDX, out, sizeof( out ), msg, sizeof( msg ), key, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_PARAM, status );

    /* Should reject NULL output buf */
    status = jolt_hash( JOLT_HASH_BLAKE2B, NULL, sizeof( out ), msg, sizeof( msg ), key, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_PARAM, status );

    /* Should reject for too large of an output buffer */
    status = jolt_hash( JOLT_HASH_BLAKE2B, out, 65, msg, sizeof( msg ), key, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_PARAM, status );

    /* Should reject NULL msg */
    status = jolt_hash( JOLT_HASH_BLAKE2B, out, sizeof( out ), NULL, sizeof( msg ), key, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_PARAM, status );

    /* Should reject 0-len msg */
    status = jolt_hash( JOLT_HASH_BLAKE2B, out, sizeof( out ), msg, 0, key, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_PARAM, status );

    /* Should reject non-0-len NULL key */
    status = jolt_hash( JOLT_HASH_BLAKE2B, out, sizeof( out ), msg, sizeof( msg ), NULL, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_PARAM, status );

    /********************************
     * Use Blake2B as a test-vector *
     ********************************/
    status = jolt_hash( JOLT_HASH_BLAKE2B, out, sizeof( out ), msg, sizeof( msg ), key, sizeof( key ) );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, out, sizeof( expected ) );
}

TEST_CASE( "jolt_hash_from_str", MODULE_NAME )
{
    jolt_hash_type_t actual;

    actual = jolt_hash_from_str( NULL );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_UNDEFINED, actual );

    actual = jolt_hash_from_str( "nonexistant" );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_UNDEFINED, actual );

    actual = jolt_hash_from_str( "blake2b" );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_BLAKE2B, actual );

    actual = jolt_hash_from_str( "sha256" );
    TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_SHA256, actual );
}
