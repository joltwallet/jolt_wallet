#include "aes132_cmd.h"
#include "aes132_comm_marshaling.h"
#include "aes132_conf.h"
#include "aes132_jolt.h"
#include "esp_log.h"
#include "setup.h"
#include "sodium.h"
#include "stdbool.h"
#include "string.h"
#include "unity.h"
#include <esp_timer.h>

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

static const char MODULE_NAME[] = "[aes132a]";
static const char TAG[] = "test_aes132";

/* Many of these tests only work on an unlocked ataes132a */

TEST_CASE("Random Buffer Fill", MODULE_NAME) {
    // Setup required hardware
    test_setup_i2c();

    uint8_t res;
    uint8_t out[100];

    // TEST 1 0 Random Bytes
    const uint8_t soln1[10] = { 0 };
    sodium_memzero(out, sizeof(out));
    res = aes132_rand(out, 0);
    TEST_ASSERT_EQUAL_UINT8(0, res);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(soln1, out, sizeof(soln1));

    // TEST 2: 5 Random Bytes
    const uint8_t soln2[10] = {0xA5,0xA5,0xA5,0xA5,0xA5};
    sodium_memzero(out, sizeof(out));
    res = aes132_rand(out, 5);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(soln2, out, sizeof(soln2));

    // TEST 3 41 Random Bytes
    const uint8_t soln3[50] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
            0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
            0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
            0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };
    sodium_memzero(out, sizeof(out));
    res = aes132_rand(out, 41);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(soln3, out, sizeof(soln3));
}

TEST_CASE("Configure Device", MODULE_NAME) {
    /* Incomplete */
    // Setup required hardware
    test_setup_i2c();

    aes132_write_chipconfig();
    aes132_write_counterconfig();
    aes132_write_keyconfig();
    aes132_write_zoneconfig();

     // todo: Make sure Legacy command fails
     // todo: Make sure DecRead or WriteCompute fails
     // todo: Make sure AuthCompute fails
}

TEST_CASE("Auth", MODULE_NAME) {
    /* Actually tests many things:
     * 1) Master Key generate/load
     * 2) KeyCreate
     * 3) Encrypt
     */
    // Setup required hardware
    test_setup_i2c();
    uint8_t res;

    /* Load the Master Key and setup random nonce */
    res = aes132_jolt_setup();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* todo: replace; just testing outbound authentication currently */
    res = aes132_auth(NULL, 0);
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
}
    /* KeyLoad (note this overwrites the KeyCreate, just used for
     * determinism); just storing here to paste in later*/
    //const uint128_t const_key = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,
    //        0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    //res = aes132_key_load( const_key, key_id );
    //TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

TEST_CASE("Key Stretch", MODULE_NAME) {
    /* Actually tests many things:
     * 1) Master Key generate/load
     * 2) KeyCreate
     * 3) Encrypt
     */
    // Setup required hardware
    test_setup_i2c();

    uint8_t res;
    const uint32_t n_iterations = 300;
    const char payload[32] = "Super Secret Data To Encrypt";

    /* Load the Master Key and setup random nonce */
    res = aes132_jolt_setup();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* KeyCreate */
    res = aes132_create_stretch_key();
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );

    /* Stretch Key */
    int64_t start = esp_timer_get_time();
    res = aes132_stretch(payload, sizeof(payload), n_iterations );
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
    int64_t end = esp_timer_get_time();
    printf("Performed %d encrypt iterations over %lld uS.\n"
            "Average time per iteration: %lld uS\n",
            n_iterations, end-start, (end-start)/n_iterations);
    TEST_ASSERT_EQUAL_HEX8( AES132_DEVICE_RETCODE_SUCCESS, res );
}

TEST_CASE("Counter Read", MODULE_NAME) {
    /* Read all counters */
    // Setup required hardware
    test_setup_i2c();

    printf("Reading all device counters:\n");
    for(uint8_t counter_id=0; counter_id < 16; counter_id++) {
        uint32_t count;
        aes132_counter(&count, counter_id);
        printf("Key %d: %u\n", counter_id, count);
    }
    printf("Read all device counters complete.\n");
}

TEST_CASE("BlockRead: Check if LockConfig", MODULE_NAME) {
    // Setup required hardware
    test_setup_i2c();
    uint8_t data;
    uint8_t res;
    res = aes132_blockread(&data, AES132_LOCK_CONFIG_ADDR, sizeof(data));
    TEST_ASSERT_EQUAL_HEX8(0x55, data);
}
