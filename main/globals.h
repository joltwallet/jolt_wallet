/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_GLOBALS_H__
#define __JOLT_GLOBALS_H__

#include "u8g2.h"
#include "menu8g2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "vault.h"

extern vault_t *vault;

extern volatile u8g2_t u8g2_obj;
extern volatile u8g2_t *u8g2;
extern volatile SemaphoreHandle_t disp_mutex;
extern volatile menu8g2_t menu_obj;
extern volatile menu8g2_t *menu;

extern volatile QueueHandle_t input_queue;
extern volatile TaskHandle_t console_h;

extern volatile bool statusbar_draw_enable_obj;
extern volatile bool *statusbar_draw_enable;

extern volatile QueueHandle_t loading_queue;
extern volatile bool loading_draw_enable;

#endif
