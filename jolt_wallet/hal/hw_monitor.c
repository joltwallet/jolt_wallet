#include "hw_monitor.h"
#include "jolt_gui/jolt_gui.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <driver/adc.h>

#include "esp_wifi.h"

#include "globals.h"
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void jolt_hw_monitor_get_battery_level(hardware_monitor_t *monitor);
static void jolt_hw_monitor_get_bluetooth_level(hardware_monitor_t *monitor);
static void jolt_hw_monitor_get_wifi_level(hardware_monitor_t *monitor);
static void jolt_hw_monitor_init();

/**********************
 *  STATIC VARIABLES
 **********************/
const char TAG[] = "hal/monitor";

/**********************
 *  DEFINE MACROS
 **********************/
#define MONITOR_UPDATE(x) \
    monitor->val = x;


void jolt_hw_monitor_task() {
    ESP_LOGI(TAG, "Starting hardware monitor task");
    jolt_hw_monitor_init();
    for( ;; ) {
        jolt_hw_monitor_update();
        vTaskDelay(pdMS_TO_TICKS(CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS));
    }
}

static void jolt_hw_monitor_get_battery_level(hardware_monitor_t *monitor) {
    /* Responds with a value between 0 and 100.
     * If a value above 100 is returned, the battery is charging.
     *
     * Cycles for demonstration purpose */
    static uint8_t level = 0;
    level = (level + 10) % 120;
    int val = adc1_get_raw(JOLT_ADC1_VBATT);
    ESP_LOGD(TAG, "vbatt: %d\n", val);
    // todo: translate this raw adc value to a percentage
    // todo; average, use this value
    MONITOR_UPDATE(level);
}

static void jolt_hw_monitor_get_bluetooth_level(hardware_monitor_t *monitor) {
    /* Returns with the bluetooth strength level. 0 if no connected. */
    static uint8_t level = 0;
    MONITOR_UPDATE(level);
}

static void jolt_hw_monitor_get_wifi_level(hardware_monitor_t *monitor) {
    /* Returns with the wifi strength level */
    uint8_t wifi_strength;
    wifi_ap_record_t ap_info;
    if(esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK){
        wifi_strength = 0;
    }
    else {
        wifi_strength = -ap_info.rssi;
    }

    MONITOR_UPDATE(wifi_strength);
}

void jolt_hw_monitor_get_lock_status(hardware_monitor_t *monitor) {
    MONITOR_UPDATE(!(vault->valid));
}

/* Creates all the hardware_monitor mutex's and sets their updater functions */
static void jolt_hw_monitor_init() {
#if 0
    for(uint8_t i=0; i < JOLT_GUI_STATUSBAR_INDEX_NUM; i++) {
        jolt_gui_store.statusbar.indicators[i].mutex = xSemaphoreCreateMutex();
    }
#endif
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_BATTERY].update = 
            &jolt_hw_monitor_get_battery_level;
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_WIFI].update = 
            &jolt_hw_monitor_get_wifi_level;
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_BLUETOOTH].update = 
            &jolt_hw_monitor_get_bluetooth_level;
    jolt_gui_store.statusbar.indicators[JOLT_GUI_STATUSBAR_INDEX_LOCK].update = 
            &jolt_hw_monitor_get_lock_status;
}

/* Iterates through all hardware_monitor's and call's their updater function */
void jolt_hw_monitor_update() {
    for(uint8_t i=0; i < JOLT_GUI_STATUSBAR_INDEX_NUM; i++) {
        jolt_gui_store.statusbar.indicators[i].update(
                &jolt_gui_store.statusbar.indicators[i]);
    }
}

