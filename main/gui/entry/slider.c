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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"
#include "sodium.h"
#include "easy_input.h"
#include "u8g2.h"
#include "menu8g2.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "../gui.h"
#include "../statusbar.h"
#include "../entry.h"
#include "../../vault.h"
#include "../../helpers.h"
#include "../../globals.h"

static const char* TAG = "entry_slider";

#define SLIDER_PADDING 10
#define SLIDER_REL_HEIGHT 0.2


bool entry_slider_callback(menu8g2_t *prev, uint8_t *output, uint8_t delta,
        const char *title, void (*callback)(uint8_t output) ){
    /* Slider entry for analogue values from attainable range from current
     * output and delta values (if current output is 250, and delta is 10,
     * it'll never get to 255)
     * Starts at position output, and goes up/down from delta*/
    menu8g2_t local_menu;
    menu8g2_t *menu = &local_menu;
    menu8g2_copy(menu, prev);
    u8g2_t *u8g2 = menu->u8g2;

    bool res;

    uint8_t maximum = 255  - ((255 - *output) % delta);

    for(;;){
        ESP_LOGD(TAG, "slider value: %d", *output);
        MENU8G2_BEGIN_DRAW(menu)
            size_t slider_width = u8g2_GetDisplayWidth(u8g2) - 2*SLIDER_PADDING;
            size_t header_height = menu8g2_buf_header(menu, title);
            size_t active_height = u8g2_GetDisplayHeight(u8g2) - header_height;
            size_t slider_height = SLIDER_REL_HEIGHT * active_height;

            size_t y = header_height + ((active_height - slider_height) / 2);

            u8g2_DrawFrame(u8g2, SLIDER_PADDING, y, slider_width, slider_height);
            u8g2_DrawBox(u8g2, SLIDER_PADDING, y, 
                    slider_width * (*output) / maximum, slider_height);
        MENU8G2_END_DRAW(menu)

        uint64_t input_buf;
        if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (1ULL << EASY_INPUT_BACK)){
                res = false;
                goto exit;
            }
            else if(input_buf & (1ULL << EASY_INPUT_UP)){
                if( *output + delta < UINT8_MAX ) {
                    *output += delta;
                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_DOWN)){
                if( *output - delta > 0 ) {
                    *output -= delta;
                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_ENTER)){
                res = true;
                goto exit;

            }
        }
        if(NULL != callback){
            callback(*output);
        }
    }

    exit:
        MENU8G2_BEGIN_DRAW(menu)
            u8g2_ClearDisplay(menu->u8g2);
        MENU8G2_END_DRAW(menu)
        return res;
}

bool entry_slider(menu8g2_t *prev, uint8_t *output, uint8_t delta, const char *title){
    return entry_slider_callback(prev, output, delta, title, NULL);
}

