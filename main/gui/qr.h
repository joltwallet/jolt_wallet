/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */


#ifndef __JOLT_QR_H__
#define __JOLT_QR_H__

#include "menu8g2.h"
#include "nano_lib.h"
#include "qrcode.h"
#include "mbedtls/bignum.h"

void display_qr(menu8g2_t *menu, uint8_t x_offset, uint8_t y_offset,
        QRCode *qrcode, uint8_t scale);
void display_qr_center(menu8g2_t *menu, QRCode *qrcode, uint8_t scale);

#endif
