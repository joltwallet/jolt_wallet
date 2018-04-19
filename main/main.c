#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "nvs.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "easy_input.h"
#include "gui.h"

#include "security.h"


void app_main(){
    QueueHandle_t *input_queue = malloc(sizeof(QueueHandle_t));
    easy_input_queue_init(input_queue);

    xTaskCreate(easy_input_push_button_task,
            "ButtonDebounce", 4096,
            (void *)input_queue, 15,
            NULL);

    // Todo: Initialize Wifi Here

    xTaskCreate(gui_task,
            "GuiTask", 16000,
            (void *)input_queue, 10,
            NULL);

	vTaskSuspend(NULL);
}
