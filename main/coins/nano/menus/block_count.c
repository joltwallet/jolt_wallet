/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <libwebsockets.h>
#include "nano_lws.h"
#include "nano_parse.h"
#include "menu8g2.h"

#include "submenus.h"
#include "../../../globals.h"
#include "../../../gui/statusbar.h"
#include "../../../gui/gui.h"

static const char TITLE[] = "Block Count";


void menu_nano_block_count(menu8g2_t *prev){
    char block_count[12];
    sprintf(block_count, "%d", nanoparse_lws_block_count());
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    for(;;){
        if(menu8g2_display_text_title(&menu, block_count, TITLE) 
                & (1ULL << EASY_INPUT_BACK)){
            goto exit;
        }
    }

    exit:
        return;
}
