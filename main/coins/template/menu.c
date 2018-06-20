/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#include "menu8g2.h"
#include "menus/submenus.h"

void menu_temp(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    const char title[] = "COIN";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 3);
    menu8g2_set_element(&elements, "Balance", &menu_temp_balance);
    menu8g2_set_element(&elements, "Send", &menu_temp_send);
    menu8g2_set_element(&elements, "Sign Message", &menu_temp_sign);
    menu8g2_create_vertical_element_menu(&menu, title, &elements);
    menu8g2_elements_free(&elements);
}
