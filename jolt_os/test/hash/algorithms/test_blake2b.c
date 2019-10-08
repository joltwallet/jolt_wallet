#include "algorithms/blake2b.h"
#include "jolt_hash.h"
#include "sodium.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_hash_blake2b]";

TEST_CASE( "blake2b basic", MODULE_NAME )
{
    jolt_hash_status_t status;
    uint8_t expected[64], out[64];
    const uint8_t msg[] = {0x00, 0x01, 0x02, 0x03};
    uint8_t key[64];

    /*************************
     * Populate Test-Vectors *
     *************************/
    /* Taken from https://github.com/BLAKE2/BLAKE2/blob/master/testvectors/blake2b-kat.txt */
    {
        /* Populate Key */
        const char hex[] = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c"
                           "2d2e2f303132333435363738393a3b3c3d3e3f";
        int res;
        res = sodium_hex2bin( key, sizeof( key ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    {
        /* Populate Expected */
        const char hex[] = "beaa5a3d08f3807143cf621d95cd690514d0b49efff9c91d24b59241ec0eefa5f60196d407048bba8d2146828e"
                           "bcb0488d8842fd56bb4f6df8e19c4b4daab8ac";
        int res;
        res = sodium_hex2bin( expected, sizeof( expected ), hex, strlen( hex ), NULL, NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Single */
    {
        status = jolt_hash( JOLT_HASH_BLAKE2B, out, sizeof( out ), msg, sizeof( msg ), key, sizeof( key ) );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
        TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, out, sizeof( expected ) );
    }

    /* Multi */
    {
        jolt_hash_t ctx = {0};
        ctx.type        = JOLT_HASH_BLAKE2B;
        ctx.hash        = out;
        ctx.hash_len    = sizeof( out );
        memcpy( ctx.key, key, sizeof( key ) );
        ctx.key_len = sizeof( key );
        status      = jolt_hash_init( &ctx );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );

        /* Actually hash some data */
        status = jolt_hash_update( &ctx, msg, 2 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
        status = jolt_hash_update( &ctx, msg + 2, 2 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );

        status = jolt_hash_final( &ctx );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
        TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, out, sizeof( expected ) );
    }
}
