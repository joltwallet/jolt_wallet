#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"

#include "nano_lib.h"
#include "qrcode.h"

#include "menu8g2.h"
#include "../vault.h"
#include "submenus.h"
#include "../globals.h"
#include "../helpers.h"
#include "../qr.h"

void menu_address_qr(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_init(&menu,
            menu8g2_get_u8g2(prev),
            menu8g2_get_input_queue(prev)
            );
    vault_rpc_t rpc;
    vault_rpc_response_t res;
    nvs_handle nvs_secret;
    QRCode qrcode;
    uint8_t qrcode_bytes[qrcode_getBufferSize(CONFIG_NANORAY_QR_VERSION)];
    uint64_t input_buf;

    //get public key
    init_nvm_namespace(&nvs_secret, "secret");
    nvs_get_u32(nvs_secret, "index", &(rpc.payload.public_key.index));
    nvs_close(nvs_secret);

    rpc.type = PUBLIC_KEY;
    res = vault_rpc(&rpc);

    if(res != RPC_SUCCESS){
        return;
    }

    nl_err_t public_to_qr(&qrcode, qrcode_bytes, 
            rpc.payload.public_key.block.account, NULL);

    //u8g2_SetContrast(u8g2, 1); // Phones have trouble with bright displays
    display_centered_qr(menu.u8g2, &qrcode, CONFIG_NANORAY_QR_SCALE);

    for(;;){
		if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf == (1ULL << EASY_INPUT_BACK)){
                // todo: Restore User's Brightness Here
                return;
            }
        }
    }
}
