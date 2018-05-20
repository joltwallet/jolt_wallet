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

#define N_LOADING_FRAMES GRAPHIC_NANO_LOAD_F
#define LOADING_FRAME_TIME_MS 50
#define LOADING_TEXT_Y 22

bool loading_draw_enable;

/* Globals */
QueueHandle_t loading_queue;

static bool previous_statusbar_draw_enable;

typedef struct loading_text_t{
    const char *title;
    const char *text;
} loading_text_t;

void loading_init(menu8g2_t *menu){
    loading_draw_enable = false;
    loading_queue = xQueueCreate( 1, sizeof(loading_text_t) );
    xTaskCreate(loading_task, "LoadingTask", 8192,
            (void *) menu, 6, NULL);
}

void loading_enable( void ){
    loading_draw_enable = true;
    previous_statusbar_draw_enable = statusbar_draw_enable;
    statusbar_draw_enable = false;
}

void loading_disable( void ){
    loading_draw_enable = false;
    statusbar_draw_enable = previous_statusbar_draw_enable;
}

void loading_text(const char *text){
    loading_text_title(text, "");
}

void loading_text_title(const char *text, const char *title){
    loading_draw_enable = true;
    loading_text_t payload = { .title = title, .text = text };
    xQueueOverwrite( loading_queue, &payload );
}

bool loading_check_cancel(menu8g2_t *menu){
    /* Returns true if a back input press is on the queue */
	uint64_t input_buf;

    while(xQueueReceive(menu->input_queue, &input_buf, 0)) {
        if(input_buf & 1ULL << EASY_INPUT_BACK){
            return true;
        }
    }
    return false;
}

void loading_task(void *menu_in){
    menu8g2_t *prev = (menu8g2_t *) menu_in;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    loading_text_t payload;
    const unsigned char *graphic = NULL;

    for(uint8_t i = 0;; i = (i+
    	    !xQueueReceive(loading_queue, &payload, 
                pdMS_TO_TICKS(LOADING_FRAME_TIME_MS))) % N_LOADING_FRAMES){

        if(!loading_draw_enable){
            i = 0;
            continue;
        }
        graphic = graphic_nano_load[i];
#if 0
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
#endif

        MENU8G2_BEGIN_DRAW(&menu)
            menu8g2_buf_header(&menu, payload.title);

            u8g2_SetDrawColor(menu.u8g2, 1);
            u8g2_DrawXBM( menu.u8g2, 
                    (u8g2_GetDisplayWidth(menu.u8g2) - GRAPHIC_NANO_LOAD_W) / 2,
                    32,
                    GRAPHIC_NANO_LOAD_W,
                    GRAPHIC_NANO_LOAD_H,
                    graphic);

            u8g2_SetFont(menu.u8g2, u8g2_font_profont12_tf);
            u8g2_DrawStr(menu.u8g2, get_center_x(menu.u8g2, payload.text),
                    LOADING_TEXT_Y, payload.text);
        MENU8G2_END_DRAW(&menu)
    }
}
