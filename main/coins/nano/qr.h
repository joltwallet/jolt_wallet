#ifndef __NANORAY_NANO_QR_H__
#define __NANORAY_NANO_QR_H__

#include "menu8g2.h"
#include "nano_lib.h"
#include "qrcode.h"
#include "mbedtls/bignum.h"

nl_err_t public_to_qr(QRCode *qrcode, uint8_t *qrcode_bytes, 
        uint256_t public_key, mbedtls_mpi *amount);

#endif
