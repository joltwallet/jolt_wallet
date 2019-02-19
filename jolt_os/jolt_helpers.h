/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_HELPERS_H__
#define __JOLT_HELPERS_H__

#include "jolttypes.h"
#include "esp_err.h"
#include "bipmnemonic.h"
#include "lvgl/lvgl.h"
#include "vault.h"

/* Macros so that the battery adc pin can be specified in Kconfig */
#define XJOLT_ADC1(x) ADC1_GPIO ## x ## _CHANNEL
#define JOLT_ADC1(x) XJOLT_ADC1(x)
#define JOLT_ADC1_VBATT JOLT_ADC1(CONFIG_JOLT_VBATT_SENSE_PIN)

#define JOLT_OS_DERIVATION_PURPOSE ((uint32_t) BM_HARDENED | 44)
#define JOLT_OS_DERIVATION_PATH ((uint32_t) BM_HARDENED | 0x4A4F4C54) /* 'J' 'O' 'L' 'T' */
static const char JOLT_OS_DERIVATION_BIP32_KEY[] = "JOLT_OS";

void nvs_log_err(esp_err_t err);
void factory_reset();
void shuffle_arr(uint8_t *arr, int arr_len);
void set_jolt_cast();

char **jolt_h_malloc_char_array(int n);
void jolt_h_free_char_array(char **arr, int n);

bool jolt_h_strcmp_suffix( char *str, char *suffix);
void jolt_h_fn_home_refresh(char *str);

void jolt_h_settings_vault_set(vault_cb_t fail_cb, vault_cb_t success_cb);

#endif
