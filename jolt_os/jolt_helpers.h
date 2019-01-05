/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_HELPERS_H__
#define __JOLT_HELPERS_H__

#include "jolttypes.h"
#include "esp_err.h"

/* Macros so that the battery adc pin can be specified in Kconfig */
#define XJOLT_ADC1(x) ADC1_GPIO ## x ## _CHANNEL
#define JOLT_ADC1(x) XJOLT_ADC1(x)
#define JOLT_ADC1_VBATT JOLT_ADC1(CONFIG_JOLT_VBATT_SENSE_PIN)

void nvs_log_err(esp_err_t err);
void factory_reset();
void shuffle_arr(uint8_t *arr, int arr_len);
void set_jolt_cast();

char **jolt_h_malloc_char_array(int n);
void jolt_h_free_char_array(char **arr, int n);

#endif
