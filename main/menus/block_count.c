#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "menu8g2.h"
#include "submenus.h"
#include "../globals.h"

#include <libwebsockets.h>
#include "nano_lws.h"
#include "nano_parse.h"

void menu_block_count(menu8g2_t *menu){
    char block_count[12];
    sprintf(block_count, "%d", get_block_count());
    
    for(;;){
        if(menu8g2_display_text(menu, block_count) == (1ULL << EASY_INPUT_BACK)){
            return;
        }
    }
}
