#ifndef JOLT_HAL_STORAGE_ATECC608A_H__
#define JOLT_HAL_STORAGE_ATECC608A_H__

#include "cryptoauthlib.h"

/********************
 * Define key slots *
 ********************/
#define ATECC608A_KEY_ID_AUTH    0
#define ATECC608A_KEY_ID_IO      1
#define ATECC608A_KEY_ID_STRETCH 2
#define ATECC608A_KEY_ID_PIN_0   3
#define ATECC608A_KEY_ID_PIN_1   4

/**********************************************
 * Config offset macros that weren't provided *
 **********************************************/
#define ATCA_I2CADDRESS_CFG_OFFSET            16
#define ATCA_COUNTMATCH_CFG_OFFSET            18
#define ATCA_CHIPMODE_CFG_OFFSET              19
#define ATCA_SLOTCONFIG_CFG_OFFSET( x )       ( 20 + 2 * x )
#define ATCA_COUNTER_CFG_OFFSET( x )          ( 52 + 8 * x )
#define ATCA_USELOCK_CFG_OFFSET               69
#define ATCA_VOLATILEKEYPERMISSION_CFG_OFFSET 69
#define ATCA_SECUREBOOT_CFG_OFFSET            70
#define ATCA_KDFLVLOC_CFG_OFFSET              72
#define ATCA_KDFLVSTR_CFG_OFFSET              73
#define ATCA_USEREXTRA_CFG_OFFSET             84
#define ATCA_USEREXTRAADD_CFG_OFFSET          85
#define ATCA_LOCKVALUE_CFG_OFFSET             86
#define ATCA_LOCKCONFIG_CFG_OFFSET            87
#define ATCA_SLOTLOCKED_CFG_OFFSET            88
#define ATCA_CHIPOPTIONS_CFG_OFFSET           90
#define ATCA_X509FORMAT_CFG_OFFSET            92
#define ATCA_KEYCONFIG_CFG_OFFSET             96

/**
 * @brief Convert an ATCA retcode to a string.
 * TODO: move to a helper function file.
 * @param[in] retcode Return code to fetch string for.
 */
const char *atca_retcode2str( uint8_t retcode );

/**
 * @brief Generate n_bytes of cryptographically strong random data.
 * @param[out] out Buffer to store random data. XORs with the currently available
 *             buffer. This is useful if combining RNG sources, and has no
 *             impact if not.
 * @param[in] n_bytes Number of random bytes to generate.
 * @return 0 on success
 */
uint8_t atecc608a_rand( uint8_t *out, const size_t n_bytes );

#endif
