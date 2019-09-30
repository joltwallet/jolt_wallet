#define LOG_LOCAL_LEVEL 4

#include "cryptoauthlib.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "unity.h"

static const char MODULE_NAME[] = "[atecc608a]";

/**
 * @brief Convert an ATCA retcode to a string.
 * TODO: move to a helper function file.
 * @param[in] retcode Return code to fetch string for.
 */
static const char *atca_retcode2str( uint8_t retcode )
{
#define STRCASE( name ) \
    case name: return #name;

    switch( retcode ) {
        STRCASE( ATCA_SUCCESS )
        STRCASE( ATCA_CONFIG_ZONE_LOCKED )
        STRCASE( ATCA_DATA_ZONE_LOCKED )
        STRCASE( ATCA_WAKE_FAILED )
        STRCASE( ATCA_CHECKMAC_VERIFY_FAILED )
        STRCASE( ATCA_PARSE_ERROR )
        STRCASE( ATCA_STATUS_CRC )
        STRCASE( ATCA_STATUS_UNKNOWN )
        STRCASE( ATCA_STATUS_ECC )
        STRCASE( ATCA_STATUS_SELFTEST_ERROR )
        STRCASE( ATCA_FUNC_FAIL )
        STRCASE( ATCA_GEN_FAIL )
        STRCASE( ATCA_BAD_PARAM )
        STRCASE( ATCA_INVALID_ID )
        STRCASE( ATCA_INVALID_SIZE )
        STRCASE( ATCA_RX_CRC_ERROR )
        STRCASE( ATCA_RX_FAIL )
        STRCASE( ATCA_RX_NO_RESPONSE )
        STRCASE( ATCA_RESYNC_WITH_WAKEUP )
        STRCASE( ATCA_PARITY_ERROR )
        STRCASE( ATCA_TX_TIMEOUT )
        STRCASE( ATCA_RX_TIMEOUT )
        STRCASE( ATCA_TOO_MANY_COMM_RETRIES )
        STRCASE( ATCA_SMALL_BUFFER )
        STRCASE( ATCA_COMM_FAIL )
        STRCASE( ATCA_TIMEOUT )
        STRCASE( ATCA_BAD_OPCODE )
        STRCASE( ATCA_WAKE_SUCCESS )
        STRCASE( ATCA_EXECUTION_ERROR )
        STRCASE( ATCA_UNIMPLEMENTED )
        STRCASE( ATCA_ASSERT_FAILURE )
        STRCASE( ATCA_TX_FAIL )
        STRCASE( ATCA_NOT_LOCKED )
        STRCASE( ATCA_NO_DEVICES )
        STRCASE( ATCA_HEALTH_TEST_ERROR )
        STRCASE( ATCA_ALLOC_FAILURE )
        default: return "<unknown_retcode>";
    }
#undef STRCASE
}

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
