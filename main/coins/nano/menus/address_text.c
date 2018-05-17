#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"
#include "esp_log.h"

#include "nano_lib.h"

#include "menu8g2.h"
#include "../../vault.h"
#include "submenus.h"
#include "../../globals.h"
#include "../../statusbar.h"

static const char* TAG = "address_text";

void menu_nano_address_text(menu8g2_t *prev){
    vault_rpc_t rpc;
    nvs_handle nvs_secret;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    bool statusbar_draw_original = statusbar_draw_enable;

    init_nvm_namespace(&nvs_secret, "secret");
    nvs_get_u32(nvs_secret, "index", &(rpc.public_key.index));
    nvs_close(nvs_secret);

    rpc.type = NANO_PUBLIC_KEY;

    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }

    char address[ADDRESS_BUF_LEN];
    nl_public_to_address(address, sizeof(address),
            rpc.public_key.block.account);

    statusbar_draw_enable = false;
    menu.post_draw = NULL;
    
    printf("Address: %s\n", address);

    for(;;){
        if(menu8g2_display_text(&menu, address) == (1ULL << EASY_INPUT_BACK)){
            statusbar_draw_enable = statusbar_draw_original;
            return;
        }
    }
}
