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


// Definitions for variables in globals.h
u8g2_t u8g2;
QueueHandle_t input_queue;
QueueHandle_t vault_queue;
SemaphoreHandle_t disp_mutex;

void app_main(){
    // Setup Input Button Debouncing Code
    easy_input_queue_init(&input_queue);
    xTaskCreate(easy_input_push_button_task,
            "ButtonDebounce", 4096,
            (void *)&input_queue, 17,
            NULL);

    // Initialize the OLED Display
    setup_screen(&u8g2);
    disp_mutex = xSemaphoreCreateMutex();
    
    // Allocate space for the vault and see if a copy exists in NVS
    vault_t vault;
    if (E_FAILURE == vault_init(&vault)){
        first_boot_menu(&vault);
    }
    
    // Initialize Wireless
    wifi_connect();

    xTaskCreate(vault_task,
            "VaultTask", 32000,
            (void *) &vault, 14,
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
