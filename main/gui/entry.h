/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */


#ifndef __JOLT_ENTRY_H__
#define __JOLT_ENTRY_H__

#define MAX_PIN_DIGITS 9

bool entry_pin(menu8g2_t *menu, unsigned char *pin_hash, const char *title);
bool entry_number_arr(menu8g2_t *prev, int8_t* num_entries, uint8_t n_digit, uint8_t n_decimal, const char *title);

bool entry_slider_callback(menu8g2_t *prev, uint8_t *output, uint8_t delta,
        const char *title, void (*callback)(uint8_t output) );
bool entry_slider(menu8g2_t *prev, uint8_t *output, uint8_t delta, const char *title);

#endif
