#ifndef __NANORAY_QR_H__
#define __NANORAY_QR_H__

#include "u8g2.h"
#include "nano_lib.h"
#include "qrcode.h"
#include "mbedtls/bignum.h"

void display_qr(u8g2_t *u8g2, uint8_t x_offset, uint8_t y_offset,
        QRCode *qrcode, uint8_t scale);

void display_centered_qr(u8g2_t *u8g2, QRCode *qrcode, uint8_t scale);

nl_err_t public_to_qr(QRCode *qrcode, uint8_t *qrcode_bytes, 
        uint256_t public_key, mbedtls_mpi *amount);

#endif
