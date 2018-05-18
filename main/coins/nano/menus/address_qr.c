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
#include "../../../statusbar.h"

void menu_nano_address_qr(menu8g2_t *prev){
    vault_rpc_t rpc;
    vault_rpc_response_t res;
    QRCode qrcode;
    uint8_t qrcode_bytes[qrcode_getBufferSize(CONFIG_NANORAY_QR_VERSION)];
    uint64_t input_buf;
    nl_err_t err;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    bool statusbar_draw_original = statusbar_draw_enable;


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
        return;
    }

    err = public_to_qr(&qrcode, qrcode_bytes, 
            rpc.nano_public_key.block.account, NULL);
    if( err != E_SUCCESS ){
        return;
    }

    statusbar_draw_enable = false;
    menu.post_draw = NULL;
    //u8g2_SetContrast(u8g2, 1); // Phones have trouble with bright displays
    display_qr_center(&menu, &qrcode, CONFIG_NANORAY_QR_SCALE);

    for(;;){
		if(xQueueReceive(menu.input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf == (1ULL << EASY_INPUT_BACK)){
                // todo: Restore User's Brightness Here
                statusbar_draw_enable = statusbar_draw_original;
                return;
            }
        }
    }
}
