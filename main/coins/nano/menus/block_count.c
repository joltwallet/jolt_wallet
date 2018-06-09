/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
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
