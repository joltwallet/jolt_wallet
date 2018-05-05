#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"

#include "nano_lib.h"

#include "menu8g2.h"
#include "../vault.h"
#include "submenus.h"
#include "../globals.h"
#include "../statusbar.h"

void menu_address_text(menu8g2_t *prev){
    vault_rpc_t rpc;
    nvs_handle nvs_secret;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    init_nvm_namespace(&nvs_secret, "secret");
    nvs_get_u32(nvs_secret, "index", &(rpc.payload.public_key.index));
    nvs_close(nvs_secret);

    rpc.type = PUBLIC_KEY;

    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }

    char address[ADDRESS_BUF_LEN];
    nl_public_to_address(address, sizeof(address),
            rpc.payload.public_key.block.account);
    statusbar_disable(&menu);
    for(;;){
        if(menu8g2_display_text(&menu, address) == (1ULL << EASY_INPUT_BACK)){
            statusbar_enable(&menu);
            return;
        }
    }
}
