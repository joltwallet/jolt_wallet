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

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"

#include "nano_lib.h"
#include "qrcode.h"

#include "menu8g2.h"
#include "../../../vault.h"
#include "submenus.h"
#include "../../../globals.h"
#include "../../../helpers.h"
#include "../../../qr.h"
#include "../qr.h"
#include "../../../statusbar.h"
#include "../../../gui.h"

void menu_nano_address_qr(menu8g2_t *prev){
    vault_rpc_t rpc;
    vault_rpc_response_t res;
    QRCode qrcode;
    uint8_t qrcode_bytes[qrcode_getBufferSize(CONFIG_JOLT_QR_VERSION)];
    uint64_t input_buf;
    nl_err_t err;

    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    //get public key
    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(rpc.nano_public_key.index))){
        rpc.nano_public_key.index = 0;
    }
    nvs_close(nvs_h);

    rpc.type = NANO_PUBLIC_KEY;
    res = vault_rpc(&rpc);

    if(res != RPC_SUCCESS){
        goto exit;
    }

    err = public_to_qr(&qrcode, qrcode_bytes, 
            rpc.nano_public_key.block.account, NULL);
    if( err != E_SUCCESS ){
        goto exit;
    }

    FULLSCREEN_ENTER(&menu)
    //u8g2_SetContrast(u8g2, 1); // Phones have trouble with bright displays
    display_qr_center(&menu, &qrcode, CONFIG_JOLT_QR_SCALE);
    for(;;){
        if(xQueueReceive(menu.input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (1ULL << EASY_INPUT_BACK)){
                // todo: Restore User's Brightness Here
                FULLSCREEN_EXIT(&menu)
                goto exit;
            }
        }
    }

    exit:
        return;
}
