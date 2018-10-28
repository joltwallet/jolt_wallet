/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_GLOBALS_H__
#define __JOLT_GLOBALS_H__

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "vault.h"

/* Macros so that the battery adc pin can be specified in Kconfig */
#define XJOLT_ADC1(x) ADC1_GPIO ## x ## _CHANNEL
#define JOLT_ADC1(x) XJOLT_ADC1(x)
#define JOLT_ADC1_VBATT JOLT_ADC1(CONFIG_JOLT_VBATT_SENSE_PIN)

extern vault_t *vault;

extern volatile SemaphoreHandle_t disp_mutex;

extern volatile QueueHandle_t input_queue;
extern volatile TaskHandle_t console_h;

#endif
