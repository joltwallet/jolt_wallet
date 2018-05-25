#include "menu8g2.h"
#include "menus/submenus.h"

void menu_nano(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    const char title[] = "Nano";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 9);
    menu8g2_set_element(&elements, "Balance", &menu_nano_balance);
    menu8g2_set_element(&elements, "Receive", &menu_nano_receive);
    menu8g2_set_element(&elements, "Send (uart)", &menu_nano_send_uart);
    menu8g2_set_element(&elements, "Send (contact)", &menu_nano_send_contact);
    menu8g2_set_element(&elements, "Update Contacts (uart)", &menu_nano_update_contact_uart);
    menu8g2_set_element(&elements, "Block Count", &menu_nano_block_count);
    menu8g2_set_element(&elements, "Select Account", &menu_nano_select_account);
    menu8g2_set_element(&elements, "Address (text)", &menu_nano_address_text);
    menu8g2_set_element(&elements, "Address (QR)", &menu_nano_address_qr);
    menu8g2_create_vertical_element_menu(&menu, title, &elements);
    menu8g2_elements_free(&elements);
}
