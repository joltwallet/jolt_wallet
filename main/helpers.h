/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_HELPERS_H__
#define __JOLT_HELPERS_H__

#include "u8g2.h"
#include "menu8g2.h"
#include "nano_lib.h"
#include "nvs.h"

void nvs_log_err(esp_err_t err);
nl_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace);
void factory_reset();
void store_mnemonic_reboot(menu8g2_t *menu, char *mnemonic);
void shuffle_arr(uint8_t *arr, int arr_len);
uint8_t get_display_brightness();
void save_display_brightness(uint8_t brightness);
void set_jolt_cast();

#endif
