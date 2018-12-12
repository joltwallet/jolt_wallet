/* Provides placeholder functions when simulating gui not on device.
 */
#if PC_SIMULATOR
#include "../lvgl/lvgl.h"
#include "../lv_conf.h"
#include "jolt_gui.h"
#include "test_stubs.h"
#include "jolt_gui_statusbar.h"

/* Lazy implementation since PC_SIMULATOR doesn't need mutexes */
SemaphoreHandle_t xSemaphoreCreateMutex() { 
    return 1;
}
BaseType_t xSemaphoreTake(SemaphoreHandle_t xSemaphore, TickType_t xTicksToWait) {
    return 1;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t xSemaphore) {
    return 1;
}

/* *********************************************
 * StatusBar Hardware Monitor Update Functions *
 * *********************************************/
#if 0
#define MONITOR_UPDATE(x) \
    xSemaphoreTake(monitor->mutex, portMAX_DELAY); \
    monitor->val = x;\
    xSemaphoreGive(monitor->mutex);
#endif
#define MONITOR_UPDATE(x) \
    monitor->val = x;

void get_battery_level(hardware_monitor_t *monitor) {
    /* Responds with a value between 0 and 100.
     * If a value above 100 is returned, the battery is charging.
     *
     * Cycles for demonstration purpose */
    static uint8_t level = 0;
    level = (level + 10) % 120;
    MONITOR_UPDATE(level);
}

void get_bluetooth_level(hardware_monitor_t *monitor) {
    /* Returns with the bluetooth strength level. 0 if no connected. */
    static uint8_t level = 0;
    level = !level;
    MONITOR_UPDATE(level);
}

void get_wifi_level(hardware_monitor_t *monitor) {
    /* Returns with the wifi strength level */
    static uint8_t i = 0;
    uint8_t levels[] = {0, 50, 70, 100};
    i = (i + 1) % sizeof(levels);
    MONITOR_UPDATE(levels[i]);
}

void get_lock_status(hardware_monitor_t *monitor) {
    static uint8_t level = 0;
    level = !level;
    MONITOR_UPDATE(level);
}

/* Creates all the hardware_monitor mutex's and sets their updater functions */
void hardware_monitors_init() {
#if 0
    for(uint8_t i=0; i < JOLT_GUI_STATUSBAR_INDEX_NUM; i++) {
        jolt_gui_store.statusbar.indicators[i].mutex = xSemaphoreCreateMutex();
    }
#endif
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_BATTERY].update = 
            &get_battery_level;
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_WIFI].update = 
            &get_wifi_level;
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_BLUETOOTH].update = 
            &get_bluetooth_level;
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_LOCK].update = 
            &get_lock_status;
}

/* Iterates through all hardware_monitor's and call's their updater function */
void hardware_monitors_update() {
    for(uint8_t i=0; i < JOLT_GUI_STATUSBAR_INDEX_NUM; i++) {
        jolt_gui_store.statusbar.indicators[i].update(
                &jolt_gui_store.statusbar.indicators[i]);
    }
}

/* ************************************
 * Generic LittlevGL button Callbacks *
 * ************************************/
lv_res_t list_release_action(lv_obj_t * list_btn) {
    /* PLACEHOLDER STUB*/
    printf("List element click:%s\n", lv_list_get_btn_text(list_btn));
    return LV_RES_OK; /*Return OK because the list is not deleted*/
}

#endif
