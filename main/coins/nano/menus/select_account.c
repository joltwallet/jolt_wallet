#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"
#include <string.h>
#include "esp_log.h"

#include "nano_lib.h"

#include "menu8g2.h"
#include "../../../vault.h"
#include "submenus.h"
#include "../../../globals.h"
#include "../../../gui.h"
#include "../../../loading.h"

#include "nano_lws.h"
#include "nano_parse.h"

static const char TAG[] = "nano_sel_acc";
static const char TITLE[] = "Nano Account";

static menu8g2_err_t get_nano_address(char buf[], size_t buf_len, const char *options[], const uint32_t index){
    char address[ADDRESS_BUF_LEN];
    vault_rpc_t rpc;
    rpc.type = NANO_PUBLIC_KEY;
    rpc.nano_public_key.index = index;
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        strlcpy(buf, "ERROR", buf_len);
        return MENU8G2_FAILURE;
    }
    else{
        nl_public_to_address(address, sizeof(address),
                rpc.nano_public_key.block.account);
        snprintf(buf, buf_len, "%d. %s", index, address);
        return MENU8G2_SUCCESS;
    }
}

void menu_nano_select_account(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    nvs_handle nvs_h;
    init_nvm_namespace(&nvs_h, "nano");
    if(ESP_OK != nvs_get_u32(nvs_h, "index", &(menu.index))){
        menu.index = 0;
    }
    nvs_close(nvs_h);

    // Dummy RPC command to prime vault cache
    vault_rpc_t rpc;
    rpc.type = NANO_PUBLIC_KEY;
    rpc.nano_public_key.index = 0;
    if(vault_rpc(&rpc) != RPC_SUCCESS){
        return;
    }

    if(menu8g2_create_vertical_menu(&menu, TITLE, NULL,
            (void *)&get_nano_address, UINT32_MAX)){
        // Enter
        init_nvm_namespace(&nvs_h, "nano");
        ESP_ERROR_CHECK(nvs_set_u32(nvs_h, "index", menu.index));
        nvs_close(nvs_h);
    }
}
