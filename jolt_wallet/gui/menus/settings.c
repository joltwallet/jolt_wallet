/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "esp_log.h"

#include "menu8g2.h"
#include "submenus.h"

#include "../loading.h"
#include "../entry.h"
#include "../../globals.h"
#include "../../helpers.h"
#include "../../vault.h"
#include "../../radio/wifi.h"
#include "../../gui/gui.h"
#include "../../hal/storage.h"


static void wifi_details(menu8g2_t *prev){
    const char title[] = "WiFi Details";
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    char new_ap_info[45];
    get_ap_info(new_ap_info, sizeof(new_ap_info));
    for(;;){
        if(menu8g2_display_text_title(&menu, new_ap_info, title)
                & (1ULL << EASY_INPUT_BACK)){
            return;
        }
    }
}

static void menu_factory_reset(menu8g2_t *prev){
    bool res;
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);

    const char title[] = "Factory Reset?";

    const char *options[] = {"No", "Yes"};
    res = menu8g2_create_simple(&menu, title, options, 2);

    if(res==false || menu.index==0){ //Pressed Back
        return;
    }

    storage_factory_reset();
}

#define SCREEN_BRIGHTNESS_DELTA 25
static void screen_brightness_callback(uint8_t brightness){
    SCREEN_MUTEX_TAKE;
    u8g2_SetContrast(u8g2, brightness);
    SCREEN_MUTEX_GIVE;
}

static void screen_brightness(menu8g2_t *menu) {
    const char title[] = "Brightness";
    uint8_t brightness = get_display_brightness();

    entry_slider_callback(menu, &brightness, SCREEN_BRIGHTNESS_DELTA, title,
                &screen_brightness_callback);
    save_display_brightness(brightness);
}

void menu_settings(menu8g2_t *prev){
    menu8g2_t menu;
    menu8g2_copy(&menu, prev);
    const char title[] = "Settings";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 4);
    menu8g2_set_element(&elements, "Screen Brightness", &screen_brightness);
    menu8g2_set_element(&elements, "WiFi Details", &wifi_details);
    menu8g2_set_element(&elements, "Bluetooth", NULL);
    menu8g2_set_element(&elements, "Factory Reset", &menu_factory_reset);
    menu8g2_create_vertical_element_menu(&menu, title, &elements);
    menu8g2_elements_free(&elements);
}
