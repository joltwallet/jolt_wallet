/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "easy_input.h"

#include "gui/gui.h"
#include "radio/wifi.h"
#include "gui/first_boot.h"
#include "vault.h"
#include "globals.h"
#include "console.h"
#include "helpers.h"
#include "syscore/filesystem.h"
#include "gui/statusbar.h"
#include "hal/i2c.h"


// Definitions for variables in globals.h
volatile u8g2_t u8g2_obj;
volatile u8g2_t *u8g2;
volatile QueueHandle_t input_queue;
volatile menu8g2_t menu_obj;
volatile menu8g2_t *menu;

static const char TAG[] = "main";

#ifndef UNIT_TESTING
// So our app_main() doesn't override the unit test app_main()
void app_main() {
    // Setup Input Button Debouncing Code
    easy_input_queue_init((QueueHandle_t *)&input_queue);
    xTaskCreate(easy_input_push_button_task,
            "ButtonDebounce", 2500,
            (void *)&input_queue, 19,
            NULL);

    // Setup and Install I2C Driver and supporting objects
    i2c_driver_setup();

    // Initialize the OLED Display
    u8g2 = &u8g2_obj;
    setup_screen((u8g2_t *) u8g2);
    u8g2_SetContrast( u8g2, get_display_brightness() );

    // Create Global Menu Object
    menu = &menu_obj;
    menu8g2_init(menu, (u8g2_t *) u8g2, input_queue, disp_mutex, NULL, statusbar_update);

    // Allocate space for the vault and see if a copy exists in NVS
    if( false == vault_setup()) {
        first_boot_menu();
    }
    
    // Initialize Wireless
    esp_log_level_set("wifi", ESP_LOG_NONE);
    set_jolt_cast();
    wifi_connect();

    xTaskCreate(gui_task,
            "GUI", 16000,
            NULL, 10,
            NULL);

    // ==== Initialize the file system ====
    filesystem_init();

    // Initiate Console
    initialize_console();
    start_console();
}
#endif
