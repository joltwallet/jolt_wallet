#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"

#include "nano_lib.h"

#include "menu8g2.h"
#include "../vault.h"
#include "submenus.h"
#include "../globals.h"

void menu_address_text(menu8g2_t *menu){
    vault_rpc_t rpc;
    vault_rpc_response_t res;
    nvs_handle nvs_secret;

    init_nvm_namespace(&nvs_secret, "secret");
    nvs_get_u32(nvs_secret, "index", &(rpc.payload.public_key.index));
    nvs_close(nvs_secret);

    rpc.type = PUBLIC_KEY;
    res = vault_rpc(&rpc);

    if(res == RPC_SUCCESS){
        char address[ADDRESS_BUF_LEN];
        nl_public_to_address(address, sizeof(address),
                rpc.payload.public_key.block.account);
        for(;;){
            if(menu8g2_display_text(menu, address) == (1ULL << EASY_INPUT_BACK)){
                return;
            }
        }
    }
}
