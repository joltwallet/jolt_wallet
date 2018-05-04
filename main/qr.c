#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "u8g2.h"
#include "qrcode.h"
#include "qr.h"
#include "nano_lib.h"
#include "mbedtls/bignum.h"

void display_qr(u8g2_t *u8g2, uint8_t x_offset, uint8_t y_offset,
        QRCode *qrcode, uint8_t scale){
	// Displays a fully formed QR Code to the OLED display
    u8g2_FirstPage(u8g2);
    do{
		for (uint8_t y = 0; y < qrcode->size; y++) {
			for (uint8_t x = 0; x < qrcode->size; x++) {
				if(scale==1){
					if (qrcode_getModule(qrcode, x, y)) {
						u8g2_DrawPixel(u8g2, x+x_offset, y+y_offset);
					}
				}
				else{
    			    if (qrcode_getModule(qrcode, x, y)) {
						u8g2_DrawBox(u8g2,
                                scale*x + x_offset,
                                scale*y + y_offset,
                                scale, scale);
					}
                }
			}
		}
    } while(u8g2_NextPage(u8g2));
}

void display_centered_qr(u8g2_t *u8g2, QRCode *qrcode, uint8_t scale){
    int16_t x_offset = (u8g2_GetDisplayWidth(u8g2) - (qrcode->size * scale))/2;
    int16_t y_offset = (u8g2_GetDisplayHeight(u8g2) - (qrcode->size * scale))/2;

    x_offset = (x_offset < 0) ? 0 : x_offset;
    y_offset = (y_offset < 0) ? 0 : y_offset;
    printf("x: %d\n", x_offset);
    printf("y: %d\n", y_offset);
    
	display_qr(u8g2, x_offset, y_offset, qrcode, scale);
}

nl_err_t public_to_qr(QRCode *qrcode, uint8_t *qrcode_bytes, 
        uint256_t public_key, mbedtls_mpi *amount){
    char buf[120];
    char address[ADDRESS_BUF_LEN];
    char amount_str[BALANCE_DEC_BUF_LEN];
    const char prefix[] = "xrb";
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

    #if CONFIG_NANORAY_QR_TYPE_SIMPLE
    strcpy(buf, address);
    strlower(buf);
    #endif

    #if CONFIG_NANORAY_QR_TYPE_STANDARD
    char *buf_moving = buf;
    strncpy(buf, prefix, strlen(prefix));
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

    #if CONFIG_NANORAY_QR_TYPE_SHORT
    strcpy(buf, toupper(address));
    if(buf[0]=='x' || buf[0]=='X'){
        buf[3] = '-';
    }
    else{
        buf[4] = '-';
    }
    #endif

    qrcode_initText(qrcode, qrcode_bytes, CONFIG_NANORAY_QR_VERSION,
            ECC_LOW, buf);
    return E_SUCCESS;
}
