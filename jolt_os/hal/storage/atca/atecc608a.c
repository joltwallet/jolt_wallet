//#define LOG_LOCAL_LEVEL 4

#include "atecc608a.h"
#include "esp_log.h"
#include "jolttypes.h"
#include "sodium.h"

const char *atca_retcode2str( uint8_t retcode )
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

uint8_t atecc608a_conf()
{
    ATCA_STATUS status;
    bool is_locked;

    /* Ensure device is unlocked */
    status = atcab_is_locked( LOCK_ZONE_CONFIG, &is_locked );
    if( ATCA_SUCCESS != status ) { goto exit; }
    if( is_locked ) {
        status = ATCA_CONFIG_ZONE_LOCKED;
        goto exit;
    }

#if 0
    {
        uint8_t cfg_data[ATCA_ECC_CONFIG_SIZE] = { 0 };

        /* TODO complete configuration */
        cfg_data[ATCA_I2CADDRESS_CFG_OFFSET] = 0x6A;
        cfg_data[ATCA_COUNTMATCH_CFG_OFFSET]; /* 1 byte; need datasheet */
        cfg_data[ATCA_CHIPMODE_CFG_OFFSET] = ATCA_CHIPMODE_I2C_ADDRESS_FLAG;
        &cfg_data[ATCA_SLOTCONFIG_CFG_OFFSET(x)];
        &cfg_data[ATCA_COUNTER_CFG_OFFSET(x)];
        cfg_data[ATCA_USELOCK_CFG_OFFSET];
        cfg_data[ATCA_VOLATILEKEYPERMISSION_CFG_OFFSET];
        cfg_data[ATCA_SECUREBOOT_CFG_OFFSET];
        cfg_data[ATCA_KDFLVLOC_CFG_OFFSET];
        cfg_data[ATCA_KDFLVSTR_CFG_OFFSET];
        cfg_data[ATCA_USEREXTRA_CFG_OFFSET];
        cfg_data[ATCA_USEREXTRAADD_CFG_OFFSET];
        cfg_data[ATCA_LOCKVALUE_CFG_OFFSET];
        cfg_data[ATCA_LOCKCONFIG_CFG_OFFSET];
        cfg_data[ATCA_SLOTLOCKED_CFG_OFFSET];
        cfg_data[ATCA_CHIPOPTIONS_CFG_OFFSET];
        cfg_data[ATCA_X509FORMAT_CFG_OFFSET];
        cfg_data[ATCA_KEYCONFIG_CFG_OFFSET];



        // First 16 bytes are not writeable
        status = atcab_write_config_zone( cfg_data );
    }
#endif

exit:
    return ATCA_STATUS_UNKNOWN;
}

uint8_t atecc608a_rand( uint8_t *out, const size_t n_bytes )
{
    if( 0 == n_bytes || NULL == out ) return ATCA_SUCCESS;

    ATCA_STATUS status;
    size_t out_head                   = 0;  // Current index into out buffer
    size_t n_copy                     = 0;  // number of bits top copy from rand buffer
    CONFIDENTIAL uint8_t rand_out[32] = {0};
    do {
        status = atcab_random( rand_out );
        if( ATCA_SUCCESS != status ) goto exit;

        // Copy over up to 32 bytes
        if( n_bytes - out_head > 32 ) { n_copy = 32; }
        else
            n_copy = n_bytes - out_head;

        for( uint8_t i = 0; i < n_copy; i++ ) { out[out_head + i] ^= rand_out[i]; }

        out_head += n_copy;
    } while( n_copy > 0 );

exit:
    sodium_memzero( rand_out, sizeof( rand_out ) );
    return status;
}
