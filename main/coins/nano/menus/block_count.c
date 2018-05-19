#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "menu8g2.h"
#include "submenus.h"
#include "../../../globals.h"
#include "../../../statusbar.h"

#include <libwebsockets.h>
#include "nano_lws.h"
#include "nano_parse.h"

static const char TITLE[] = "Block Count";

void menu_nano_block_count(menu8g2_t *prev){
    char block_count[12];
    sprintf(block_count, "%d", get_block_count());
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    bool statusbar_draw_original = statusbar_draw_enable;

    statusbar_draw_enable = false;
    menu.post_draw = NULL;
    for(;;){
        if(menu8g2_display_text_title(&menu, block_count, TITLE) 
                == (1ULL << EASY_INPUT_BACK)){
            statusbar_draw_enable = statusbar_draw_original;
            return;
        }
    }
}
