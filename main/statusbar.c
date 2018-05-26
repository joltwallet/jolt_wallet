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
#define STATUSBAR_ANCHOR_X 82
#define STATUSBAR_PAD 2

static const char* TAG = "StatusBar";

/* Globals */
bool statusbar_draw_enable = true;

static void statusbar_pending(menu8g2_t *menu){
    const uint16_t x = STATUSBAR_ANCHOR_X;
    const uint16_t y = 1;
    bool pending_status = false; // todo: check websocket

    if (pending_status){
        u8g2_SetDrawColor(menu->u8g2, 0);
        u8g2_DrawBox( menu->u8g2, x, y,
                GRAPHIC_PENDING_W, GRAPHIC_PENDING_H);
        u8g2_SetDrawColor(menu->u8g2, 1);
        u8g2_DrawXBM( menu->u8g2, x, y,
                GRAPHIC_PENDING_W,
                GRAPHIC_PENDING_H,
                graphic_pending);
    }
}

static void statusbar_wifi(menu8g2_t *menu){
    /* Call in a drawing loop */
    const uint16_t x = STATUSBAR_ANCHOR_X + GRAPHIC_PENDING_W 
            + 1 * STATUSBAR_PAD;
    const uint16_t y = 1;
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

static void statusbar_bluetooth(menu8g2_t *menu){
    const uint16_t x = STATUSBAR_ANCHOR_X + GRAPHIC_PENDING_W + GRAPHIC_WIFI_W 
            + 2 * STATUSBAR_PAD;
    const uint16_t y = 0;
    bool bluetooth_status = true; // todo: read bluetooth status

    if (bluetooth_status){
        u8g2_SetDrawColor(menu->u8g2, 0);
        u8g2_DrawBox( menu->u8g2, x, y,
                GRAPHIC_BLUETOOTH_W, GRAPHIC_BLUETOOTH_H);
        u8g2_SetDrawColor(menu->u8g2, 1);
        u8g2_DrawXBM( menu->u8g2, x, y,
                GRAPHIC_BLUETOOTH_W,
                GRAPHIC_BLUETOOTH_H,
                graphic_bluetooth);
    }
}

static void statusbar_battery(menu8g2_t *menu){
    const uint16_t x = STATUSBAR_ANCHOR_X + GRAPHIC_PENDING_W + GRAPHIC_WIFI_W 
            + GRAPHIC_BLUETOOTH_W + 3 * STATUSBAR_PAD;
    const uint16_t y = 1;
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
    u8g2_DrawBox( menu->u8g2, x, y, GRAPHIC_BATTERY_W, GRAPHIC_BATTERY_H);
    u8g2_SetDrawColor(menu->u8g2, 1);
    u8g2_DrawXBM( menu->u8g2, x, y,
            GRAPHIC_BATTERY_W,
            GRAPHIC_BATTERY_H,
            graphic);
}

void statusbar_update(menu8g2_t *menu){
    statusbar_pending(menu);
    statusbar_wifi(menu);
    statusbar_bluetooth(menu);
    statusbar_battery(menu);
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
