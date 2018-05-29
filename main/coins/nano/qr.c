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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "u8g2.h"
#include "qrcode.h"
#include "qr.h"
#include "nano_lib.h"
#include "mbedtls/bignum.h"

nl_err_t public_to_qr(QRCode *qrcode, uint8_t *qrcode_bytes, 
        uint256_t public_key, mbedtls_mpi *amount){
    char buf[120];
    char address[ADDRESS_BUF_LEN];
    char amount_str[BALANCE_DEC_BUF_LEN];
    size_t olen;
    nl_err_t err;

    err = nl_public_to_address(address, sizeof(address), public_key);
    if(err != E_SUCCESS){
        return err;
    }

    if (amount==NULL){
        strcpy(amount_str, "0");
    }
    else{
        mbedtls_mpi_write_string(amount, 10,
                amount_str, sizeof(amount_str), &olen);
    }

    #if CONFIG_JOLT_QR_TYPE_SIMPLE
    strcpy(buf, address);
    strlower(buf);
    #endif

    #if CONFIG_JOLT_QR_TYPE_STANDARD
    char *buf_moving = buf;
    strncpy(buf, CONFIG_NANO_LIB_ADDRESS_PREFIX,
            strlen(CONFIG_NANO_LIB_ADDRESS_PREFIX)-1);
    buf_moving += strlen(prefix);

    *buf_moving = ':';
    buf_moving++;

    strcpy(buf_moving, address);
    buf_moving += strlen(address);

    strcpy(buf_moving, "?amount=");
    buf_moving += 8;

    strcpy(buf_moving, amount_str);
    buf_moving += strlen(str_amount);
    #endif

    #if CONFIG_JOLT_QR_TYPE_SHORT
    strcpy(buf, toupper(address));
    if(buf[0]=='x' || buf[0]=='X'){
        buf[3] = '-';
    }
    else{
        buf[4] = '-';
    }
    #endif

    qrcode_initText(qrcode, qrcode_bytes, CONFIG_JOLT_QR_VERSION,
            ECC_LOW, buf);
    return E_SUCCESS;
}
