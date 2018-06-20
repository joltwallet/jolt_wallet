/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/portmacro.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "graphics.h"

#include "statusbar.h"
#include "../radio/wifi.h"
#include "../globals.h"

#define STATUSBAR_UPDATE_PERIOD_MS 2000
#define STATUSBAR_PAD 2

static const char* TAG = "StatusBar";


/* Globals */
volatile bool statusbar_draw_enable = true;

static void statusbar_pending(menu8g2_t *menu, uint16_t *x, uint16_t *y){
    bool pending_status = false; // todo: check websocket

    if (pending_status){
        *x -= (GRAPHIC_PENDING_W + STATUSBAR_PAD);
        u8g2_SetDrawColor(menu->u8g2, 0);
        u8g2_DrawBox( menu->u8g2, *x, *y,
                GRAPHIC_PENDING_W, GRAPHIC_PENDING_H);
        u8g2_SetDrawColor(menu->u8g2, 1);
        u8g2_DrawXBM( menu->u8g2, *x, *y,
                GRAPHIC_PENDING_W,
                GRAPHIC_PENDING_H,
                graphic_pending);
    }
}

static void statusbar_wifi(menu8g2_t *menu, uint16_t *x, uint16_t *y){
    /* Call in a drawing loop */
    *x -= (GRAPHIC_WIFI_W + STATUSBAR_PAD);
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
            *x += (GRAPHIC_WIFI_W + STATUSBAR_PAD);
            return;
    }
    u8g2_SetDrawColor(menu->u8g2, 0);
    u8g2_DrawBox( menu->u8g2, *x, *y, GRAPHIC_WIFI_W, GRAPHIC_WIFI_H);
    u8g2_SetDrawColor(menu->u8g2, 1);
    u8g2_DrawXBM( menu->u8g2, *x, *y,
            GRAPHIC_WIFI_W,
            GRAPHIC_WIFI_H,
            graphic);
}

static void statusbar_bluetooth(menu8g2_t *menu, uint16_t *x, uint16_t *y){
    bool bluetooth_status = false; // todo: read bluetooth status

    if (bluetooth_status){
        *x -= (GRAPHIC_BLUETOOTH_W + STATUSBAR_PAD);
        u8g2_SetDrawColor(menu->u8g2, 0);
        u8g2_DrawBox( menu->u8g2, *x, *y,
                GRAPHIC_BLUETOOTH_W, GRAPHIC_BLUETOOTH_H);
        u8g2_SetDrawColor(menu->u8g2, 1);
        u8g2_DrawXBM( menu->u8g2, *x, *y,
                GRAPHIC_BLUETOOTH_W,
                GRAPHIC_BLUETOOTH_H,
                graphic_bluetooth);
    }
}

static void statusbar_battery(menu8g2_t *menu, uint16_t *x, uint16_t *y){
    *x -= (GRAPHIC_BATTERY_W + STATUSBAR_PAD);
    const unsigned char *graphic;
    uint8_t battery_strength = 3; //todo: read battery voltage
    switch(battery_strength){
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
    u8g2_SetDrawColor(menu->u8g2, 0);
    u8g2_DrawBox( menu->u8g2, *x, *y, GRAPHIC_BATTERY_W, GRAPHIC_BATTERY_H);
    u8g2_SetDrawColor(menu->u8g2, 1);
    u8g2_DrawXBM( menu->u8g2, *x, *y,
            GRAPHIC_BATTERY_W,
            GRAPHIC_BATTERY_H,
            graphic);
}

void statusbar_update(menu8g2_t *menu){
    uint16_t x = u8g2_GetDisplayWidth(menu->u8g2);
    uint16_t y = 2;
    // Graphical order right to left
    statusbar_battery(menu, &x, &y);
    statusbar_bluetooth(menu, &x, &y);
    statusbar_wifi(menu, &x, &y);
    statusbar_pending(menu, &x, &y);
}

void statusbar_task(void *menu_in){
    menu8g2_t *menu = (menu8g2_t *) menu_in;

    ESP_LOGI(TAG, "Starting StatusBar Task");

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
