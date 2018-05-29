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
