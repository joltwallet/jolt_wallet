#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "menu8g2.h"

#include "gui.h"
#include "menus/settings.h"

static void setup_screen(u8g2_t *u8g2){
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

void gui_task(void *input_queue){
    /* Master GUI Task */
    u8g2_t u8g2;
    setup_screen(&u8g2);

    menu8g2_t menu;
    menu8g2_init(&menu, &u8g2, *(QueueHandle_t *)input_queue);

    bool main_menu_response;

    const char title[] = "Main Menu";
    const char *options[] = {
        "Receive",          // 0
        "Block Count",      // 1
        "Balance",          // 2
        "Select Account",   // 3
        "Address (text)",   // 4
        "Addresss (QR)",    // 5
        "Seed (text)",      // 6
        "Settings"          // 7
    };
    for(;;){
        main_menu_response = menu8g2_create_simple(&menu, title, options, 8);
        if(main_menu_response==false){
            continue;
        }
        switch(menu8g2_get_index(&menu)){
            case 7:
                break;
            default:
                break;
        }
    }

    vTaskDelete(NULL); // Should never reach here!
}
