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

static const char* TAG = "entry_number";

#define NUMBER_ENTRY_SPACING 10


#if 0
double number_entry_double(menu8g2_t *prev, uint8_t n_digit, uint8_t n_decimal, const char *title){
    /* Returns the user entered double; promps with n_digit digits with the
     * decimal point n_decimal from the right */

    // Convert entries into a double
    // Compute Decimal; Starts at digit right of decimal
    double j = 0.1;
    for(int i = n_digit-n_decimal+1; i <= n_digit; i++, j*=0.1){


    }
    // Compute integer; Starts at digit right of decimal
    for(int i = 0; i < n_digit; i++){
        // Set Background color for position selection
        if( i == n_digit - n_decimal ){
            continue
            u8g2_SetDrawColor(u8g2, 1);
            strlcpy(buf, ".", sizeof(buf));
        }
    }

}
#endif

bool entry_number_arr(menu8g2_t *prev, int8_t* output, uint8_t n_digit, uint8_t n_decimal, const char *title){
    /* Fills in output with user entries.
     * decimal point n_decimal from the right */
    menu8g2_t local_menu;
    menu8g2_t *menu = &local_menu;
    menu8g2_copy(menu, prev);

    bool res;

    u8g2_t *u8g2 = menu->u8g2;
    u8g2_SetFont(u8g2, u8g2_font_profont12_tf);
    u8g2_uint_t title_height = u8g2_GetAscent(u8g2) + CONFIG_MENU8G2_BORDER_SIZE;
    u8g2_SetFont(u8g2, u8g2_font_profont17_tf);
    int8_t entry_pos = 0; // which element the user is currently entering
    uint16_t num_height = u8g2_GetAscent(u8g2);
    uint8_t border = (u8g2_GetDisplayWidth(u8g2) - 
            (NUMBER_ENTRY_SPACING * (n_digit+1))) / 2;

    uint64_t input_buf;
    char buf[24];
    int8_t *num_entries = calloc( n_digit, sizeof(int8_t) );

    FULLSCREEN_ENTER(menu);
    for(;;){
        MENU8G2_BEGIN_DRAW(menu)
            u8g2_SetFont(u8g2, u8g2_font_profont12_tf);
            u8g2_DrawStr(u8g2, menu8g2_get_center_x(menu, title), title_height,
                    title);
            u8g2_DrawHLine(u8g2, 0, title_height+1, u8g2_GetDisplayWidth(u8g2));

            u8g2_SetFont(u8g2, u8g2_font_profont17_tf);

            for(int i = 0; i < n_digit; i++){
                // Set Background color for position selection
                if(i==entry_pos){
                    u8g2_SetDrawColor(u8g2, 1);
                    u8g2_DrawBox(u8g2,
                            border + ((i + (i>=n_digit-n_decimal)) * NUMBER_ENTRY_SPACING) - 1,
                            ((u8g2_GetDisplayHeight(u8g2) - num_height)/2) - 1,
                            u8g2_GetStrWidth(u8g2, "0") + 2,
                            num_height + 2
                            );
                    u8g2_SetDrawColor(u8g2, 0);
                }
                else{
                    u8g2_SetDrawColor(u8g2, 1);
                }
                sprintf(buf, "%d", num_entries[i]);
                u8g2_DrawStr(u8g2, border + 
                            ((i + (i>=n_digit-n_decimal)) * NUMBER_ENTRY_SPACING),
                        (u8g2_GetDisplayHeight(u8g2) + num_height)/2 ,
                        buf);
            }
            // Draw decimal point
            u8g2_SetDrawColor(u8g2, 1);
            u8g2_DrawStr(u8g2, border + ((n_digit-n_decimal) * NUMBER_ENTRY_SPACING),
                    (u8g2_GetDisplayHeight(u8g2) + num_height)/2 ,
                    ".");

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
                if(++num_entries[entry_pos]>9){
                    num_entries[entry_pos] = 0;
                }

            }
            else if(input_buf & (1ULL << EASY_INPUT_DOWN)){
                if(--num_entries[entry_pos]<0){
                    num_entries[entry_pos] = 9;
                }
            }
            else if(input_buf & (1ULL << EASY_INPUT_ENTER)){
                if(entry_pos < n_digit-1){
                    entry_pos++;
                }
                else{
                    ESP_LOGI(TAG, "User finished entering number.");
                    memcpy(output, num_entries, n_digit);
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
        free(num_entries);
        return res;
}

