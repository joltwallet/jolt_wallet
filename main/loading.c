#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "nvs.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "menu8g2.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "easy_input.h"
#include "gui.h"
#include "globals.h"

#include "vault.h"
#include "nano_lib.h"
#include "first_boot.h"
#include "wifi.h"
#include "nano_lws.h"
#include "graphics.h"
#include "helpers.h"
#include "loading.h"

#define N_LOADING_FRAMES 4
#define LOADING_FRAME_TIME_MS 700
#define LOADING_TEXT_Y 12

bool loading_draw_enable;

/* Globals */
QueueHandle_t loading_queue;

void loading_init(menu8g2_t *menu){
    loading_draw_enable = false;
    loading_queue = xQueueCreate( 1, sizeof(char *) );
    xTaskCreate(loading_task, "LoadingTask", 8192,
            (void *) menu, 19, NULL);
}

void loading_disable( void ){
    loading_draw_enable = false;
}

void loading_text(char *text){
    loading_draw_enable = true;
    xQueueOverwrite(loading_queue, &text);
}

void loading_task(void *menu_in){
    menu8g2_t *prev = (menu8g2_t *) menu_in;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    menu.post_draw = NULL;

    char *text = '\0';
    const unsigned char *graphic = NULL;

    for(uint8_t i = 0;; i = (i+
    	    !xQueueReceive(loading_queue, &text, 
                pdMS_TO_TICKS(LOADING_FRAME_TIME_MS))) % N_LOADING_FRAMES){

        if(!loading_draw_enable){
            continue;
        }

        // todo: use actual loading graphic
        switch(i){
            case 0:
                graphic = graphic_battery_0;
                break;
            case 1:
                graphic = graphic_battery_1;
                break;
            case 2:
                graphic = graphic_battery_2;
                break;
            case 3:
                graphic = graphic_battery_3;
                break;
            default:
                graphic = graphic_battery_0;
                break;
        }

        MENU8G2_BEGIN_DRAW(&menu)
            u8g2_SetDrawColor(menu.u8g2, 1);
            u8g2_DrawXBM( menu.u8g2, 100, 40, // todo: better location
                    GRAPHIC_BATTERY_W,
                    GRAPHIC_BATTERY_H,
                    graphic);

            u8g2_SetFont(menu.u8g2, u8g2_font_profont12_tf);
            u8g2_DrawStr(menu.u8g2, get_center_x(menu.u8g2, text),
                    LOADING_TEXT_Y, text);
        MENU8G2_END_DRAW(&menu)
    }
}
