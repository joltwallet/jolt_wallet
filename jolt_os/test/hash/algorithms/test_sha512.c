#include "algorithms/sha512.h"
#include "jolt_hash.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_hash_sha512]";

TEST_CASE( "sha512 basic", MODULE_NAME )
{
    /* From https://www.di-mgt.com.au/sha_testvectors.html */

    jolt_hash_status_t status;
    uint8_t expected[64], out[64];
    const uint8_t msg[] = "abc"; /* Don't hash the NULL-terminator */

    {
        /* Populate Expected */
        const char hex[] = "ddaf35a193617aba cc417349ae204131 12e6fa4e89a97ea2 0a9eeee64b55d39a 2192992a274fc1a8 36ba3c23a3feebbd 454d4423643ce80e 2a9ac94fa54ca49f";
        int res;
        res = sodium_hex2bin( expected, sizeof( expected ), hex, strlen( hex ), " ", NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Single */
    {
        status = jolt_hash( JOLT_HASH_SHA512, out, sizeof( out ), msg, 3, NULL, 0 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
        TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, out, sizeof( expected ) );
    }

    /* Multi */
    {
        jolt_hash_t ctx = {0};
        ctx.type        = JOLT_HASH_SHA512;
        ctx.hash        = out;
        ctx.hash_len    = sizeof( out );
        status          = jolt_hash_init( &ctx );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );

        /* Actually hash some data */
        status = jolt_hash_update( &ctx, msg, 2 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
        status = jolt_hash_update( &ctx, msg + 2, 1 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );

        status = jolt_hash_final( &ctx );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
        TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, out, sizeof( expected ) );
    }
}
