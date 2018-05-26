#ifndef __JOLT_GLOBALS_H__
#define __JOLT_GLOBALS_H__

#include "u8g2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "globals.h"


extern volatile u8g2_t u8g2;
extern volatile QueueHandle_t vault_queue;
extern volatile QueueHandle_t input_queue;
extern volatile SemaphoreHandle_t disp_mutex;
extern QueueHandle_t backend_queue;

extern volatile bool statusbar_draw_enable;

extern volatile QueueHandle_t loading_queue;
extern volatile bool loading_draw_enable;

#endif
