/**
 * A (as far as I know) novel RLE-like encoding algorithm to tackle the
 * shortcomings of tradditional RLE.
 */

#include "jolt_helpers.h"
#include "syscore/encoding.h"
#include "unity.h"

static const char MODULE_NAME[] = "[syscore/encoding/jrle]";

TEST_CASE( "encoding & decoding", MODULE_NAME )
{
    int len;
    uint8_t buf[512] = {0};
    /* Basic Use-Case 1 */
    {
        const char test_str[]            = "AAAAbbbCDEFGHIJJJJ";
        const uint8_t expected_encoded[] = {4 | 0x80, 'A', 3 | 0x80, 'b', 7,   'C',      'D',
                                            'E',      'F', 'G',      'H', 'I', 4 | 0x80, 'J'};

        len = jolt_encoding_jrle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_jrle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        buf[len] = '\0';
        TEST_ASSERT_EQUAL_STRING( test_str, buf );
        printf( "passed %d\n", 1 );
    }

    /* Basic Use-Case 2 */
    {
        const char test_str[]            = "abbcd";
        const uint8_t expected_encoded[] = {1, 'a', 2 | 0x80, 'b', 2, 'c', 'd'};

        /* Basicdd Use-Case 1 */
        len = jolt_encoding_jrle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_jrle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        buf[len] = '\0';
        TEST_ASSERT_EQUAL_STRING( test_str, buf );
        printf( "passed %d\n", 2 );
    }

    /* Basic Use-Case 3 */
    {
        const char test_str[]            = "abcdd";
        const uint8_t expected_encoded[] = {3, 'a', 'b', 'c', 2 | 0x80, 'd'};

        len = jolt_encoding_jrle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_jrle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        buf[len] = '\0';
        TEST_ASSERT_EQUAL_STRING( test_str, buf );
        printf( "passed %d\n", 3 );
    }

    /* Repeating exceeding 127 1 */
    {
        /* 140 'a's*/
        const char test_str[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                "aaaaaaaaaaaaaaaaaaaaaaaaaaaabc";
        const uint8_t expected_encoded[] = {127 | 0x80, 'a', 13 | 0x80, 'a', 2, 'b', 'c'};

        len = jolt_encoding_jrle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_jrle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        buf[len] = '\0';
        TEST_ASSERT_EQUAL_STRING( test_str, buf );
        printf( "passed %d\n", 3 );
    }

    /* Repeating exceeding 127 2 */
    {
        /* 140 'a's*/
        const char test_str[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                "aaaaaaaaaaaaaaaaaaaaaaaaaaaabb";
        const uint8_t expected_encoded[] = {127 | 0x80, 'a', 13 | 0x80, 'a', 2 | 0x80, 'b'};

        len = jolt_encoding_jrle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_jrle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        buf[len] = '\0';
        TEST_ASSERT_EQUAL_STRING( test_str, buf );
        printf( "passed %d\n", 3 );
    }

    /* Repeating exceeding 127 2 */
    {
        /* 140 'a's*/
        const char test_str[] = "caaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                                "aaaaaaaaaaaaaaaaaaaaaaaaaaaabb";
        const uint8_t expected_encoded[] = {1, 'c', 127 | 0x80, 'a', 13 | 0x80, 'a', 2 | 0x80, 'b'};

        len = jolt_encoding_jrle_encode( buf, sizeof( buf ), (uint8_t*)test_str, strlen( test_str ) );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( expected_encoded, buf, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( sizeof( expected_encoded ), len );
        memzero( buf, sizeof( buf ) );

        len = jolt_encoding_jrle_decode( buf, sizeof( buf ), expected_encoded, sizeof( expected_encoded ) );
        TEST_ASSERT_EQUAL_INT( strlen( test_str ), len );
        buf[len] = '\0';
        TEST_ASSERT_EQUAL_STRING( test_str, buf );
        printf( "passed %d\n", 3 );
    }

    /* Auto Testing */
    for( uint32_t i = 0; i < 100; i++ ) {
        uint8_t rand_data[100];
        uint8_t encoded[2000], decoded[2000];
        esp_fill_random( rand_data, sizeof( rand_data ) );
        len = jolt_encoding_jrle_encode( encoded, sizeof( encoded ), rand_data, sizeof( rand_data ) );
        TEST_ASSERT_GREATER_THAN( 0, len );
        len = jolt_encoding_jrle_decode( decoded, sizeof( decoded ), encoded, len );
        TEST_ASSERT_EQUAL( sizeof( rand_data ), len );
        TEST_ASSERT_EQUAL_HEX8_ARRAY( rand_data, decoded, sizeof( rand_data ) );
    }
}
