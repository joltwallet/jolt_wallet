/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_GLOBALS_H__
#define __JOLT_GLOBALS_H__

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
//#include "vault.h"

//extern vault_t *vault;

extern volatile SemaphoreHandle_t disp_mutex;

extern volatile QueueHandle_t input_queue;
extern volatile TaskHandle_t console_h;

extern volatile bool statusbar_draw_enable_obj;
extern volatile bool *statusbar_draw_enable;

extern volatile QueueHandle_t loading_queue;
extern volatile bool loading_draw_enable;

#endif
