#if PC_SIMULATOR
#ifndef __JOLT_GUI_TEST_STUBS_H__
#define __JOLT_GUI_TEST_STUBS_H__
#include "../lvgl/lvgl.h"
#include "jolt_gui.h"

/* Define some FreeRTOS variables */
typedef void* SemaphoreHandle_t;
typedef uint32_t BaseType_t;
typedef BaseType_t TickType_t;
#define portMAX_DELAY 0xFFFFFFFF
SemaphoreHandle_t xSemaphoreCreateMutex( void );
BaseType_t xSemaphoreTake( SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait );
BaseType_t xSemaphoreGive( SemaphoreHandle_t xSemaphore );


/* *********************************************
 * StatusBar Hardware Monitor Update Functions *
 * *********************************************/
void get_battery_level(hardware_monitor_t *monitor);
void get_bluetooth_level(hardware_monitor_t *monitor);
void get_wifi_level(hardware_monitor_t *monitor);
void get_lock_status(hardware_monitor_t *monitor);

void hardware_monitors_init();
void hardware_monitors_update();

/* ************************************
 * Generic LittlevGL button Callbacks *
 * ************************************/
lv_res_t list_release_action(lv_obj_t * list_btn);

#endif
#endif
