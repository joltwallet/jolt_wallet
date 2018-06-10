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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "nvs.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "u8g2.h"
#include "menu8g2.h"
#include "easy_input.h"
#include "nano_lib.h"
#include "nano_lws.h"

#include "gui/gui.h"
#include "radio/wifi.h"
#include "gui/first_boot.h"
#include "vault.h"
#include "globals.h"
#include "console.h"
#include "helpers.h"


// Definitions for variables in globals.h
volatile u8g2_t u8g2;
volatile QueueHandle_t input_queue;
volatile QueueHandle_t vault_queue;
volatile SemaphoreHandle_t disp_mutex;
QueueHandle_t backend_queue;

void app_main(){
    // Setup Input Button Debouncing Code
    easy_input_queue_init((QueueHandle_t *)&input_queue);
    xTaskCreate(easy_input_push_button_task,
            "ButtonDebounce", 2500,
            (void *)&input_queue, 15,
            NULL);

    // Initialize the OLED Display
    setup_screen((u8g2_t *) &u8g2);
    u8g2_SetContrast( &u8g2, get_display_brightness() );
    disp_mutex = xSemaphoreCreateMutex();
    
    // Allocate space for the vault and see if a copy exists in NVS
    vault_t vault;
    if (E_FAILURE == vault_init(&vault)){
        first_boot_menu();
    }
    
    // Initialize Wireless
    set_jolt_cast();
    wifi_connect();
    
    // Initiate Console
    initialize_console();
    start_console();

    xTaskCreate(vault_task,
            "Vault", 16000,
            (void *) &vault, 16,
            NULL);
    
    xTaskCreate(gui_task,
            "GUI", 16000,
            NULL, 10,
            NULL);

    xTaskCreate(network_task,
            "Network", 3200,
            NULL, 5,
            NULL);

    vTaskSuspend(NULL);
}
