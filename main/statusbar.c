#include "graphics.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"

#include "wifi.h"
#include "menu8g2.h"
#include "u8g2.h"
#include "globals.h"
#include "statusbar.h"

#define STATUSBAR_UPDATE_PERIOD_MS 2000

static const char* TAG = "StatusBar";

/* Globals */
bool statusbar_draw_enable = true;

static void statusbar_wifi(menu8g2_t *menu){
    /* Call in a drawing loop */
    const uint16_t x = 100;
    const uint16_t y = 0;
    const unsigned char *graphic;
    uint8_t wifi_strength = get_wifi_strength();
    switch(wifi_strength){
        case 1:
            graphic = graphic_wifi_1;
            break;
        case 2:
            graphic = graphic_wifi_2;
            break;
        case 3:
            graphic = graphic_wifi_3;
            break;
        default:
            // Draw Nothing
            return;
    }
    u8g2_SetDrawColor(menu->u8g2, 0);
    u8g2_DrawBox( menu->u8g2, x, y, GRAPHIC_WIFI_W, GRAPHIC_WIFI_H);
    u8g2_SetDrawColor(menu->u8g2, 1);
    u8g2_DrawXBM( menu->u8g2, x, y,
            GRAPHIC_WIFI_W,
            GRAPHIC_WIFI_H,
            graphic);
}

void statusbar_update(menu8g2_t *menu){
    statusbar_wifi(menu);
}

void statusbar_task(void *menu_in){
    menu8g2_t *menu = (menu8g2_t *) menu_in;

    for(;; vTaskDelay(pdMS_TO_TICKS(STATUSBAR_UPDATE_PERIOD_MS))) {
        // Not a mutex or taskSuspend to prevent deadlock where statusbar_task 
        // is suspended while taking disp_mutex.
        if(statusbar_draw_enable){
            xSemaphoreTake(menu->disp_mutex, portMAX_DELAY);

            statusbar_update(menu);
            u8g2_SendBuffer(menu->u8g2);

            xSemaphoreGive(menu->disp_mutex);
        }
    }

    vTaskDelete(NULL); // Should never reach here!
}
