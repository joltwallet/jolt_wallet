#ifndef __NANORAY_GLOBALS_H__
#define __NANORAY_GLOBALS_H__

#include "u8g2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "globals.h"


extern u8g2_t u8g2;
extern QueueHandle_t vault_queue;
extern QueueHandle_t input_queue;
extern SemaphoreHandle_t disp_mutex;

extern bool statusbar_draw_enable;

extern QueueHandle_t loading_queue;
extern bool loading_draw_enable;

#endif
