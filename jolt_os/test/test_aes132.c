#include "sdkconfig.h"

#if CONFIG_JOLT_STORE_ATAES132A

    #include <esp_timer.h>
    #include "aes132_cmd.h"
    #include "aes132_comm_marshaling.h"
    #include "aes132_conf.h"
    #include "aes132_jolt.h"
    #include "esp_log.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/queue.h"
    #include "freertos/task.h"
    #include "sodium.h"
    #include "stdbool.h"
    #include "string.h"
    #include "unity.h"

static const char MODULE_NAME[] = "[aes132a]";
static const char TAG[]         = "test_aes132";

/**
 * Many of these tests only work on an unlocked ataes132a.
 * Locking action is independently tested.
 */

TEST_CASE( "Random Buffer Fill", MODULE_NAME )
{
    uint8_t res;
    uint8_t out[100];

    // TEST 1 0 Random Bytes
    const uint8_t soln1[10] = {0};
    sodium_memzero( out, sizeof( out ) );
    res = aes132_rand( out, 0 );
    TEST_ASSERT_EQUAL_UINT8( 0, res );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( soln1, out, sizeof( soln1 ) );

    // TEST 2: 5 Random Bytes
    const uint8_t soln2[10] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    sodium_memzero( out, sizeof( out ) );
    res = aes132_rand( out, 5 );
    TEST_ASSERT_EQUAL_UINT8( 0, res );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( soln2, out, sizeof( soln2 ) );

    // TEST 3 41 Random Bytes
    const uint8_t soln3[50] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
                               0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
                               0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    sodium_memzero( out, sizeof( out ) );
    res = aes132_rand( out, 41 );
    TEST_ASSERT_EQUAL_UINT8( 0, res );
    TEST_ASSERT_EQUAL_UINT8_ARRAY( soln3, out, sizeof( soln3 ) );
}

TEST_CASE( "Configure Device", MODULE_NAME )
{
    uint128_t data = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

    uint8_t res;

    res = aes132_write_chipconfig();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    res = aes132_write_counterconfig();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    res = aes132_write_keyconfig();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    res = aes132_write_zoneconfig();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* Make sure Legacy command fails, excpet on stretch key */
    {
        printf( "Testing key slot %d\n", AES132_KEY_ID_MASTER );
        res = aes132_legacy( AES132_KEY_ID_MASTER, data );
        TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_KEY_ERROR, res );

        printf( "Testing key slot %d\n", AES132_KEY_ID_STRETCH );
        res = aes132_legacy( AES132_KEY_ID_STRETCH, data );
        TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

        for( uint8_t i = AES132_KEY_ID_PIN( 0 ); i < 16; i++ ) {
            printf( "Testing key slot %d\n", i );
            res = aes132_legacy( i, data );
            TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_KEY_ERROR, res );
        }
    }

    TEST_FAIL_MESSAGE( "Incomplete Test" );
    // todo: Make sure DecRead or WriteCompute fails
    // todo: Make sure AuthCompute fails
}

TEST_CASE( "Load Key/Attempt Key", MODULE_NAME )
{
    /* Actually tests many things:
     * 1) Master Key generate/load
     * 2) KeyCreate
     * 3) Encrypt
     */
    uint8_t res;

    /* Load the Master Key and setup random nonce */
    res = aes132_jolt_setup();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* Load PIN Keys */
    // Some constant dummy
    const uint128_t const_key = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    uint256_t pred_secret     = {0};
    // Hash it to simulate a pin entry
    uint256_t pin_entry_hash;
    crypto_generichash_blake2b_state hs;
    crypto_generichash_init( &hs, NULL, 32, 32 );
    crypto_generichash_update( &hs, const_key, sizeof( const_key ) );
    crypto_generichash_final( &hs, pin_entry_hash, 32 );

    /* KeyCreate Stretch */
    res = aes132_create_stretch_key();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* Stretch Key */
    {
        const uint32_t n_iterations = 100;
        int64_t start               = esp_timer_get_time();
        res = aes132_stretch( (uint8_t *)pin_entry_hash, sizeof( pin_entry_hash ), n_iterations, NULL );
        TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
        int64_t diff = esp_timer_get_time() - start;
        diff /= 1000;
        if( diff > UINT32_MAX ) diff = UINT32_MAX;
        printf( "Performed %u encrypt iterations over %u mS.\n"
                "Average time per iteration: %u mS\n",
                n_iterations, (uint32_t)diff, (uint32_t)diff / n_iterations );
        TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
    }

    res = aes132_pin_load_keys( pin_entry_hash );
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    const uint128_t zeros = {0};
    uint32_t counter      = 0;

    /* Bad PIN attempt */
    res = aes132_pin_attempt( zeros, &counter, pred_secret );
    ESP_LOGD( TAG, "Counter Value: %d", counter );
    if( counter >= AES132_CUM_COUNTER_MAX ) { ESP_LOGI( TAG, "Device Deactivated; key use completely exhausted." ); }
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_MAC_ERROR, res );

    /* Good PIN attempt */
    res = aes132_pin_attempt( pin_entry_hash, &counter, pred_secret );
    ESP_LOGD( TAG, "Counter Value: %d", counter );
    if( counter >= AES132_CUM_COUNTER_MAX ) { ESP_LOGI( TAG, "Device Deactivated; key use completely exhausted." ); }
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
}

TEST_CASE( "Key Stretch", MODULE_NAME )
{
    /* Actually tests many things:
     * 1) Master Key generate/load
     * 2) KeyCreate
     * 3) Encrypt
     */

    uint8_t res;
    const uint32_t n_iterations = 300;
    char payload[32]            = "Super Secret Data To Encrypt";

    /* Load the Master Key and setup random nonce */
    res = aes132_jolt_setup();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* KeyCreate Stretch */
    res = aes132_create_stretch_key();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* Stretch Key */
    int64_t start = esp_timer_get_time();
    res           = aes132_stretch( (uint8_t *)payload, sizeof( payload ), n_iterations, NULL );
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
    int64_t diff = esp_timer_get_time() - start;
    diff /= 1000;
    if( diff > UINT32_MAX ) diff = UINT32_MAX;
    printf( "Performed %u encrypt iterations over %u mS.\n"
            "Average time per iteration: %u mS\n",
            n_iterations, (uint32_t)diff, (uint32_t)diff / n_iterations );
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
}

TEST_CASE( "BlockRead: Check if LockConfig", MODULE_NAME )
{
    uint8_t data;
    uint8_t res;
    res = aes132_blockread( &data, AES132_LOCK_CONFIG_ADDR, sizeof( data ) );
    TEST_ASSERT_EQUAL_UINT8( 0, res );
    TEST_ASSERT_EQUAL_HEX8( 0x55, data );
}
#endif
