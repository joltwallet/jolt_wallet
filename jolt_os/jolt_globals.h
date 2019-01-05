/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef JOLT_GLOBALS_H__
#define JOLT_GLOBALS_H__

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "vault.h"
#include "hal/lv_drivers/display/SSD1306.h"

vault_t *vault;

extern TaskHandle_t console_h;

#endif
