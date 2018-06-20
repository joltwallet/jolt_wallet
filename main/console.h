/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

void initialize_console();
void menu_console(menu8g2_t *prev);
bool console_check_range_argc(uint8_t argc, uint8_t min, uint8_t max);
bool console_check_equal_argc(uint8_t argc, uint8_t expected);
volatile TaskHandle_t *start_console();

#endif
