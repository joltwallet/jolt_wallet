#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "nvs.h"
#include "esp_log.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"
#include "menu8g2.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "easy_input.h"
#include "gui.h"
#include "globals.h"

#include "vault.h"
#include "nano_lib.h"
#include "first_boot.h"
#include "wifi.h"
#include "nano_lws.h"

#include "console.h"


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
            "ButtonDebounce", 4096,
            (void *)&input_queue, 17,
            NULL);

    // Initialize the OLED Display
    setup_screen((u8g2_t *) &u8g2);
    disp_mutex = xSemaphoreCreateMutex();
    
    // Allocate space for the vault and see if a copy exists in NVS
    vault_t vault;
    if (E_FAILURE == vault_init(&vault)){
        first_boot_menu();
    }
    
    // Initialize Wireless
    wifi_connect();
    
    // Initiate Console
    initialize_console();

    xTaskCreate(vault_task,
            "VaultTask", 50000,
            (void *) &vault, 14,
            NULL);
    
    xTaskCreate(backend_task,
                "BackendTask", 32000,
                NULL, 11,
                NULL);

    xTaskCreate(gui_task,
            "GuiTask", 32000,
            NULL, 10,
            NULL);

    xTaskCreate(network_task,
            "NetworkTask", 4800,
            NULL, 10,
            NULL);

    vTaskSuspend(NULL);
}
