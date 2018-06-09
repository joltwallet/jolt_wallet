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

static const char* TAG = "entry_pin";

#define PIN_SPACING 13


bool entry_pin(menu8g2_t *prev, unsigned char *pin_hash, const char *title){
    /* Screen for Pin Entry 
     * Saves results into pin_entries*/
    menu8g2_t local_menu;
    menu8g2_t *menu = &local_menu;
    menu8g2_copy(menu, prev);

    bool res;

    u8g2_t *u8g2 = menu->u8g2;
    uint8_t max_pos = MAX_PIN_DIGITS;
    u8g2_SetFont(u8g2, u8g2_font_profont12_tf);
    u8g2_uint_t title_height = u8g2_GetAscent(u8g2) + CONFIG_MENU8G2_BORDER_SIZE;
    u8g2_SetFont(u8g2, u8g2_font_profont17_tf);
    int8_t entry_pos = 0; // which element the user is currently entering
    uint8_t border = (u8g2_GetDisplayWidth(u8g2) - 
            (PIN_SPACING * max_pos)) / 2;

    uint64_t input_buf;
    char buf[24];
    int8_t pin_entries[MAX_PIN_DIGITS] = { 0 };

    FULLSCREEN_ENTER(menu);
    for(;;){
        MENU8G2_BEGIN_DRAW(menu)
            u8g2_SetFont(u8g2, u8g2_font_profont12_tf);
            u8g2_DrawStr(u8g2, menu8g2_get_center_x(menu, title), title_height,
                    title);
            u8g2_DrawHLine(u8g2, 0, title_height+1, u8g2_GetDisplayWidth(u8g2));

            u8g2_SetFont(u8g2, u8g2_font_profont17_tf);
            uint16_t num_height = u8g2_GetAscent(u8g2);

            for(int i = 0; i < max_pos; i++){
                // Set Background color for position selection
                if(i==entry_pos){
                    u8g2_SetDrawColor(u8g2, 1);
                    u8g2_DrawBox(u8g2,
                            border + (i * PIN_SPACING) - 1,
                            ((u8g2_GetDisplayHeight(u8g2) - num_height)/2) - 1,
                            u8g2_GetStrWidth(u8g2, "0") + 2,
                            num_height + 2
                            );
                    u8g2_SetDrawColor(u8g2, 0);
                }
                else{
                    u8g2_SetDrawColor(u8g2, 1);
                }
                sprintf(buf, "%d", pin_entries[i]);
                u8g2_DrawStr(u8g2, border + (i * PIN_SPACING),
                        (u8g2_GetDisplayHeight(u8g2) + num_height)/2 ,
                        buf);
            }
        MENU8G2_END_DRAW(menu)

        u8g2_SetFont(u8g2, u8g2_font_profont12_tf);
        u8g2_SetDrawColor(u8g2, 1); // Set it back to default background

        if(xQueueReceive(menu->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (1ULL << EASY_INPUT_BACK)){
                if(entry_pos>0){
                    entry_pos--;
                }
                else{
                    ESP_LOGI(TAG, "User exiting (back) pin entry screen.");
                    res = false;
                    goto exit;

                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_UP)){
                if(++pin_entries[entry_pos]>9){
                    pin_entries[entry_pos] = 0;
                }

            }
            else if(input_buf & (1ULL << EASY_INPUT_DOWN)){
                if(--pin_entries[entry_pos]<0){
                    pin_entries[entry_pos] = 9;
                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_ENTER)){
                if(entry_pos < max_pos-1){
                    entry_pos++;
                }
                else{
                    ESP_LOGI(TAG, "User entered pin; processing...");

                    // Convert pin into a 256-bit key
                    crypto_generichash_blake2b_state hs;
                    crypto_generichash_init(&hs, NULL, 32, 32);
                    crypto_generichash_update(&hs, 
                            (unsigned char *) pin_entries, MAX_PIN_DIGITS);
                    crypto_generichash_final(&hs, pin_hash, 32);

                    res = true;
                    goto exit;
                }
            }
        }
    }

    exit:
        MENU8G2_BEGIN_DRAW(menu)
            u8g2_ClearDisplay(menu->u8g2);
        MENU8G2_END_DRAW(menu)

        FULLSCREEN_EXIT(menu);
        return res;
}

