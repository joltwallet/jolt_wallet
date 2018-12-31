/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_HELPERS_H__
#define __JOLT_HELPERS_H__

#include "jolttypes.h"

void nvs_log_err(esp_err_t err);
void factory_reset();
void shuffle_arr(uint8_t *arr, int arr_len);
uint8_t get_display_brightness();
void save_display_brightness(uint8_t brightness);
void set_jolt_cast();

char **jolt_h_malloc_char_array(int n);
void jolt_h_free_char_array(char **arr, int n);

#endif
