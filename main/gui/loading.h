/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_LOADING_H__
#define __JOLT_LOADING_H__

void loading_init(menu8g2_t *menu);
void loading_disable();
void loading_enable();
void loading_task(void *menu_in);
void loading_text(const char *text);
void loading_text_title(const char *text, const char *title);

bool loading_check_cancel(menu8g2_t *menu);

#endif
