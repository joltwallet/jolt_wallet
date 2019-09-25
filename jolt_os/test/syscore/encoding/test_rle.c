#include "jolt_helpers.h"
#include "syscore/encoding.h"
#include "unity.h"

static const char MODULE_NAME[] = "[syscore/encoding/rle]";

TEST_CASE( "encoding & decoding", MODULE_NAME )
{
    int len;
    uint8_t buf[512] = {0};

    /* Basic Use-Case 1 */
    {
        const char test_str[]            = "WWWWWWWWWWWWBWWWWWWWWWWWWBBBWWWWWWWWWWWWWWWWWWWWWWWWBWWWWWWWWWWWWWW";
        const uint8_t expected_encoded[] = {12, 'W', 1, 'B', 12, 'W', 3, 'B', 24, 'W', 1, 'B', 14, 'W'};

        len = jolt_encoding_rle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_MEMORY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_rle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_MEMORY( test_str, buf, strlen( test_str ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        memzero( buf, sizeof( buf ) );
    }

    /* Basic Use-Case 2*/
    {
        const char test_str[]            = "abc";
        const uint8_t expected_encoded[] = {1, 'a', 1, 'b', 1, 'c'};

        len = jolt_encoding_rle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_MEMORY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_rle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_MEMORY( test_str, buf, strlen( test_str ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        memzero( buf, sizeof( buf ) );
    }

    /* Repeating exceeding 255 */
    {
        const char test_str[] =
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaabc";
        const uint8_t expected_encoded[] = {255, 'a', 25, 'a', 1, 'b', 1, 'c'};

        len = jolt_encoding_rle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_MEMORY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_rle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_MEMORY( test_str, buf, strlen( test_str ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        memzero( buf, sizeof( buf ) );
    }

    /* Auto Testing */
    for( uint32_t i = 0; i < 1000; i++ ) {
        uint8_t rand_data[1000];
        uint8_t encoded[2000], decoded[2000];
        esp_fill_random( rand_data, sizeof( rand_data ) );
        len = jolt_encoding_rle_encode( encoded, sizeof( encoded ), rand_data, sizeof( rand_data ) );
        TEST_ASSERT_GREATER_THAN( 0, len );
        len = jolt_encoding_rle_decode( decoded, sizeof( decoded ), encoded, len );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( rand_data, decoded, sizeof( rand_data ) );
        TEST_ASSERT_EQUAL( sizeof( rand_data ), len );
    }
}
