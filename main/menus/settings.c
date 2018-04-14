#include "menu8g2.h"
#include "submenus.h"

void settings_menu(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_init(&menu,
            menu8g2_get_u8g2(prev),
            menu8g2_get_input_queue(prev)
            );
    const char title[] = "Settings";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 3);
    menu8g2_set_element(&elements, "Screen Brightness", NULL);
    menu8g2_set_element(&elements, "Charge PoW", NULL);
    menu8g2_set_element(&elements, "Factory Reset", NULL);
    menu8g2_create_vertical_element_menu(&menu, title, &elements);
    menu8g2_elements_free(&elements);
}
