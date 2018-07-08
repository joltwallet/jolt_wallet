/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_QR_H__
#define __JOLT_QR_H__

#include "menu8g2.h"
#include "qrcode.h"
#include "mbedtls/bignum.h"

void display_qr(menu8g2_t *menu, uint8_t x_offset, uint8_t y_offset,
        QRCode *qrcode, uint8_t scale);
void display_qr_center(menu8g2_t *menu, QRCode *qrcode, uint8_t scale);

#endif
