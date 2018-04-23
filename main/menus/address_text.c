#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"

#include "nano_lib.h"

#include "menu8g2.h"
#include "security.h"
#include "submenus.h"
#include "globals.h"

void menu_address_text(menu8g2_t *prev){
    nl_err_t err;
    menu8g2_t menu;
    menu8g2_init(&menu,
            menu8g2_get_u8g2(prev),
            menu8g2_get_input_queue(prev)
            );
    vault_rpc_t rpc;
    vault_rpc_t *rpc_p = &rpc;
    vault_rpc_response_t res;
    rpc.type = PUBLIC_KEY;
    rpc.response_queue = xQueueCreate( 1, sizeof(res) );

    xQueueSend( vault_queue, (void *) &rpc_p, 0);
    xQueueReceive(rpc.response_queue, (void *) &res, portMAX_DELAY);

    if(res == RPC_SUCCESS){
        char address[ADDRESS_BUF_LEN];
        err = nl_public_to_address(address, sizeof(address),
                rpc.payload.block.account);
	    menu8g2_display_text(&menu, address);
    }

}
