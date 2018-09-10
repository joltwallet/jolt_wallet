#include "unity.h"
#include "setup.h"
#include "aes132_comm_marshaling.h"
#include "aes132_conf.h"
#include "sodium.h"
#include "stdbool.h"

static const char MODULE_NAME[] = "[aes132a]";

/* Many of these tests only work on an unlocked ataes132a */

TEST_CASE("Random Buffer Fill", MODULE_NAME) {
    // Setup required hardware
    test_setup_i2c();

    uint8_t res;
    uint8_t out[100];

    // TEST 1 0 Random Bytes
    const uint8_t soln1[10] = { 0 };
    sodium_memzero(out, sizeof(out));
    res = aes132m_rand(out, 0);
    TEST_ASSERT_EQUAL_UINT8(0, res);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(soln1, out, sizeof(soln1));

    // TEST 2: 5 Random Bytes
    const uint8_t soln2[10] = {0xA5,0xA5,0xA5,0xA5,0xA5};
    sodium_memzero(out, sizeof(out));
    res = aes132m_rand(out, 5);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(soln2, out, sizeof(soln2));

    // TEST 3 41 Random Bytes
    const uint8_t soln3[50] = { 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
            0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
            0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5,
            0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5, 0xA5 };
    sodium_memzero(out, sizeof(out));
    res = aes132m_rand(out, 41);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(soln3, out, sizeof(soln3));
}

TEST_CASE("Bitfield Config", MODULE_NAME) {
    /* Configuration for some zone.
     * Makes sure that lsb are at the top of the bitfield, msb are at bottom.
     * Bitfields are non-portable, but they make a lot of configuration much
     * more explicit. A bunch of other code is nonportable, anyways.
     *
     * Checks if the bitfields of zoneconfig match expected hexidecimal.
     * Also checks to make sure all the bitfields are expected size*/
    aes132_zoneconfig_t zoneconfig = {
        .auth_read =  true,
        .auth_write = true,
        .enc_read = false,
        .enc_write = false,
        .write_mode = 0b01,
        .use_serial = true,
        .use_small = false,
        .read_id = 0x7,
        .auth_id = 0x1,
        .volatile_transfer_ok = false,
        .write_id = 0x8,
        .read_only = AES132_ZONECONFIG_READONLY_RW,
    };
    const uint8_t soln[] = { 0b01010011, 0x17, 0x80, 0x55 };
    TEST_ASSERT_EQUAL(4, sizeof(zoneconfig));
    TEST_ASSERT_EQUAL_HEX8_ARRAY(&soln, &zoneconfig, sizeof(zoneconfig));

    TEST_ASSERT_EQUAL(1, sizeof(aes132_chipconfig_t));
    TEST_ASSERT_EQUAL(2, sizeof(aes132_counterconfig_t));
    TEST_ASSERT_EQUAL(4, sizeof(aes132_keyconfig_t));
    TEST_ASSERT_EQUAL(4, sizeof(aes132_zoneconfig_t));
}

TEST_CASE("Configure Device", MODULE_NAME) {
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

TEST_CASE("Counter Read", MODULE_NAME) {
    /* Read all counters */
    // Setup required hardware
    test_setup_i2c();

    printf("Reading all device counters:\n");
    for(uint8_t counter_id=0; counter_id < AES132_NUM_ZONES; counter_id++) {
        uint32_t count;
        aes132m_counter(&count, counter_id);
        printf("Key %d: %u\n", counter_id, count);
    }
    printf("Read all device counters complete.\n");
}

TEST_CASE("BlockRead: Check if LockConfig", MODULE_NAME) {
    // Setup required hardware
    test_setup_i2c();
    uint8_t data;
    uint8_t res;
    res =  aes132m_blockread(&data, AES132_LOCKCONFIG_ADDR, sizeof(data));
    TEST_ASSERT_EQUAL_HEX8(0x55, data);
}

TEST_CASE("PIN Auth", MODULE_NAME) {
    // Setup required hardware
    test_setup_i2c();

    aes132_write_chipconfig();
    aes132_write_counterconfig();
    aes132_write_keyconfig();
    aes132_write_zoneconfig();

    //todo: actually do the Auth Command
}

TEST_CASE("MAC Computation", MODULE_NAME) {
    uint8_t res;
    uint8_t nonce[12] = { 0 };
    res = aes132m_nonce_sync(nonce);

    // Compare ESP32 computed nonce with what the ataes132a produces
    // todo
}

#if 0
TEST_CASE("Encrypt/Decrypt", MODULE_NAME) {
    /* Tests Key 
     * Tests on the volatile key slot*/

    // Setup required hardware
    test_setup_i2c();

    uint8_t res;
    uint8_t out[100];
    const char test1[] = "Jolt Wallet 123";
    const char key1[] = "meow";

}
#endif
