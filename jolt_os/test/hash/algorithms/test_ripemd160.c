#include "jolt_hash.h"
#include "string.h"
#include "unity.h"

static const char MODULE_NAME[] = "[jolt_hash_ripemd160]";

TEST_CASE( "ripemd160 basic", MODULE_NAME )
{
    /* From https://homes.esat.kuleuven.be/~bosselae/ripemd160.html */

    jolt_hash_status_t status;
    uint8_t expected[20], out[20];
    const uint8_t msg[] = "abc"; /* Don't hash the NULL-terminator */

    {
        /* Populate Expected */
        const char hex[] = "8eb208f7e05d987a9b044a8e98c6b087f15a0bfc";
        int res;
        res = sodium_hex2bin( expected, sizeof( expected ), hex, strlen( hex ), " ", NULL, NULL );
        TEST_ASSERT_EQUAL_INT( 0, res );
    }

    /* Single */
    {
        status = jolt_hash( JOLT_HASH_RIPEMD160, out, sizeof( out ), msg, 3, NULL, 0 );
        TEST_ASSERT_EQUAL_UINT8( JOLT_HASH_STATUS_SUCCESS, status );
        TEST_ASSERT_EQUAL_UINT8_ARRAY( expected, out, sizeof( expected ) );
    }

    /* Multi */
    {
        jolt_hash_t ctx = {0};
        ctx.type        = JOLT_HASH_RIPEMD160;
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
