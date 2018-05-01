#include "menu8g2.h"
#include "../vault.h"
#include "submenus.h"
#include "../globals.h"
#include "../wifi.h"


static void menu_factory_reset_confirm(menu8g2_t *prev){
    bool res;
    vault_rpc_t rpc;
    menu8g2_t menu;
    menu8g2_init(&menu,
            menu8g2_get_u8g2(prev),
            menu8g2_get_input_queue(prev)
            );

    const char title[] = "Are you sure?";

    const char *options[] = {"No", "Yes"};
    res = menu8g2_create_simple(&menu, title, options, 2);

    if(res==false || menu.index==0){ //Pressed Back
        return;
    }

    rpc.type = FACTORY_RESET;
    vault_rpc(&rpc);
}

static void wifi_settings(menu8g2_t *prev){
    
    bool res;
    vault_rpc_t rpc;
    menu8g2_t menu;
    menu8g2_init(&menu,
                 menu8g2_get_u8g2(prev),
                 menu8g2_get_input_queue(prev)
                 );
    char new_ap_info[45];
    get_ap_info(new_ap_info);
    for(;;){
        if(menu8g2_display_text(&menu, new_ap_info) == (1ULL << EASY_INPUT_BACK)){
            return;
        }
    }
}

void menu_settings(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_init(&menu,
            menu8g2_get_u8g2(prev),
            menu8g2_get_input_queue(prev)
            );
    const char title[] = "Settings";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 4);
    menu8g2_set_element(&elements, "Screen Brightness", NULL);
    menu8g2_set_element(&elements, "WiFi", &wifi_settings);
    menu8g2_set_element(&elements, "Bluetooth", NULL);
    menu8g2_set_element(&elements, "Charge PoW", NULL);
    menu8g2_set_element(&elements, "Factory Reset", &menu_factory_reset_confirm);
    menu8g2_create_vertical_element_menu(&menu, title, &elements);
    menu8g2_elements_free(&elements);
}
