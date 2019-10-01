#define LOG_LOCAL_LEVEL 4

#include "atecc608a.h"
#include "cryptoauthlib.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "unity.h"

static const char MODULE_NAME[] = "[atecc608a]";

/**
 * @brief Unity test for ATCA return codes
 * @param[in] expected Expected return code.
 * @param[in] actual Actual return code.
 */
#define TEST_ASSERT_EQUAL_ATCA_RETCODE( expected, actual )                                          \
    {                                                                                               \
        uint8_t actual_proxy = (uint8_t)actual;                                                     \
        TEST_ASSERT_EQUAL_HEX8_MESSAGE( expected, actual_proxy, atca_retcode2str( actual_proxy ) ); \
    }

TEST_CASE( "rand", MODULE_NAME )
{
    uint8_t retcode;
    // TODO move this into general init in main.c
    ATCAIfaceCfg cfg = {.iface_type            = ATCA_I2C_IFACE,
                        .devtype               = ATECC608A,
                        .atcai2c.slave_address = 0XC0, /* Default I2C Address */
                        .atcai2c.bus           = CONFIG_JOLT_I2C_MASTER_NUM,
                        .atcai2c.baud          = CONFIG_JOLT_I2C_MASTER_FREQ_HZ,
                        .wake_delay            = 1500,
                        .rx_retries            = 20};
    atcab_init( &cfg );

    uint8_t rand_out[32] = {0};
    retcode              = atcab_random( rand_out );
    TEST_ASSERT_EQUAL_ATCA_RETCODE( ATCA_SUCCESS, retcode );
    // TODO check rand_out
}
