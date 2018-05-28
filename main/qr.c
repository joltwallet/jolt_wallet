#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu8g2.h"
#include "qrcode.h"
#include "qr.h"
#include "nano_lib.h"
#include "mbedtls/bignum.h"

void display_qr(menu8g2_t *menu, uint8_t x_offset, uint8_t y_offset,
        QRCode *qrcode, uint8_t scale){
    // Displays a fully formed QR Code to the OLED display
   
    MENU8G2_BEGIN_DRAW(menu)
        for (uint8_t y = 0; y < qrcode->size; y++) {
            for (uint8_t x = 0; x < qrcode->size; x++) {
                if(scale==1){
                    if (qrcode_getModule(qrcode, x, y)) {
                        u8g2_DrawPixel(menu->u8g2, x+x_offset, y+y_offset);
                    }
                }
                else{
                    if (qrcode_getModule(qrcode, x, y)) {
                        u8g2_DrawBox(menu->u8g2,
                                scale*x + x_offset,
                                scale*y + y_offset,
                                scale, scale);
                    }
                }
            }
        }
    MENU8G2_END_DRAW(menu)
}

void display_qr_center(menu8g2_t *menu, QRCode *qrcode, uint8_t scale){
    int16_t x_offset = (u8g2_GetDisplayWidth(menu->u8g2) - (qrcode->size * scale))/2;
    int16_t y_offset = (u8g2_GetDisplayHeight(menu->u8g2) - (qrcode->size * scale))/2;

    x_offset = (x_offset < 0) ? 0 : x_offset;
    y_offset = (y_offset < 0) ? 0 : y_offset;
    
    display_qr(menu, x_offset, y_offset, qrcode, scale);
}
