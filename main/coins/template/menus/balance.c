/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

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


static const char TAG[] = "temp_balance";
static const char TITLE[] = "temp action";

void menu_nano_balance(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    
    /***************
     * Get Balance *
     ***************/

     /******************
     * Display Balance *
     *******************/
   
    for(;;){
        if(menu8g2_display_text_title(&menu, balance_string, "temp action")
                & (1ULL << EASY_INPUT_BACK)){
            return;
        }
    }
}
