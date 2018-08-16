/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_ENTRY_H__
#define __JOLT_ENTRY_H__

#include "jolttypes.h"

#define MAX_PIN_DIGITS 9

bool entry_pin(menu8g2_t *menu, uint256_t pin_hash, const char *title);
bool entry_verify_pin(menu8g2_t *prev, uint256_t pin_hash);
bool entry_number_arr(menu8g2_t *prev, int8_t* num_entries, uint8_t n_digit, uint8_t n_decimal, const char *title);

bool entry_slider_callback(menu8g2_t *prev, uint8_t *output, uint8_t delta,
        const char *title, void (*callback)(uint8_t output) );
bool entry_slider(menu8g2_t *prev, uint8_t *output, uint8_t delta, const char *title);

#endif
