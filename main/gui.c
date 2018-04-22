#include "nvs.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "menu8g2.h"

#include "globals.h"
#include "graphics.h"
#include "gui.h"
#include "security.h"

#include "menus/submenus.h"


void setup_screen(u8g2_t *u8g2){
    // Initialize OLED Screen I2C params
	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda  = CONFIG_NANORAY_DISPLAY_PIN_SDA;
	u8g2_esp32_hal.scl  = CONFIG_NANORAY_DISPLAY_PIN_SCL;
    u8g2_esp32_hal.reset = CONFIG_NANORAY_DISPLAY_PIN_RST;
	u8g2_esp32_hal_init(u8g2_esp32_hal);
   
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(
		u8g2,
		U8G2_R0,
		u8g2_esp32_i2c_byte_cb,
		u8g2_esp32_gpio_and_delay_cb
    );  // init u8g2 structure

    // Note: SCREEN_ADDRESS is already shifted left by 1
	u8x8_SetI2CAddress(&(u8g2->u8x8), CONFIG_NANORAY_DISPLAY_ADDRESS);

	u8g2_InitDisplay(u8g2);
	u8g2_SetPowerSave(u8g2, 0); // wake up display
    u8g2_ClearDisplay(u8g2);
	u8g2_ClearBuffer(u8g2);

    u8g2_SetContrast(u8g2, 255);
}

void gui_task(){
    /* Master GUI Task */
    nl_err_t err;
	nvs_handle nvs_user;
    uint8_t boot_splash_enable = CONFIG_NANORAY_DEFAULT_BOOT_SPLASH_ENABLE;
    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, input_queue);

    // display boot_splash if option is set
    err = init_nvm_namespace(&nvs_user, "user");
    if(E_SUCCESS == err){
        nvs_get_u8(nvs_user, "boot_splash", &boot_splash_enable);
        nvs_close(nvs_user);
    }
    nvs_close(nvs_user);
    if(boot_splash_enable){
	    boot_splash( menu.u8g2 );
    }

    const char title[] = "Main";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 8);
    menu8g2_set_element(&elements, "Receive", NULL);
    menu8g2_set_element(&elements, "Block Count", NULL);
    menu8g2_set_element(&elements, "Balance", NULL);
    menu8g2_set_element(&elements, "Select Account", NULL);
    menu8g2_set_element(&elements, "Address (text)", NULL);
    menu8g2_set_element(&elements, "Address (QR)", NULL);
    menu8g2_set_element(&elements, "Seed (text)", NULL);
    menu8g2_set_element(&elements, "Settings", &menu_settings);

    for(;;){
        menu8g2_create_vertical_element_menu(&menu, title, &elements);
        menu.index = 0;
    }

    menu8g2_elements_free(&elements);
    vTaskDelete(NULL); // Should never reach here!
}
