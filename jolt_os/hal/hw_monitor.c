#include "hw_monitor.h"
#include "jolt_gui/jolt_gui.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <driver/adc.h>

#include "esp_wifi.h"

#include "jolt_globals.h"

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

/* Responds with a value between 0 and 100.
 * If a value above 100 is returned, the battery is charging.
 *
 * Cycles for demonstration purpose */
static void jolt_hw_monitor_get_battery_level(hardware_monitor_t *monitor) {
    static uint16_t vals[CONFIG_JOLT_VBATT_AVG_WINDOW] = { 0 }; // store moving average
    static uint8_t index = 0;
#if 0
    MONITOR_UPDATE(75);
    return;
#endif

    // todo: check charging gpio

    // Get a new reading
    index = (index + 1) % CONFIG_JOLT_VBATT_AVG_WINDOW;
    vals[index] = (uint16_t)adc1_get_raw(JOLT_ADC1_VBATT);
    vals[index] = 100;

    // compute average
    uint32_t cum = 0;
    uint8_t n = 0;
    for(uint8_t i=0; i < CONFIG_JOLT_VBATT_AVG_WINDOW; i++) {
        if( vals[i] != 0 ) {
            cum += vals[i];
            n++;
        }
    }
    uint32_t avg = cum / n;

    /* Note: this calculation represents a linear voltage drop assumption. */
    uint8_t percentage = (uint8_t)(100*(avg - CONFIG_JOLT_VBATT_MIN) / (CONFIG_JOLT_VBATT_MAX - CONFIG_JOLT_VBATT_MIN));
    if(percentage > 100) {
        percentage = 100;
    }

    ESP_LOGD(TAG, "[vbatt] raw: %d   percent: %d\n", avg, percentage);
    
    MONITOR_UPDATE(percentage);
}

static void jolt_hw_monitor_get_bluetooth_level(hardware_monitor_t *monitor) {
    /* Returns with the bluetooth strength level. 0 if no connected. */
    static uint8_t level;
#if CONFIG_BT_ENABLED
    level = 1; // Todo; real code to check number of connected clients
#else
    level = 0;
#endif
    MONITOR_UPDATE(level);
}

static void jolt_hw_monitor_get_wifi_level(hardware_monitor_t *monitor) {
    /* Returns with the wifi strength level */
    uint8_t wifi_strength;
#if CONFIG_CONFIG_NO_BLOBS
    wifi_ap_record_t ap_info;
    if(esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK){
        wifi_strength = 0;
    }
    else {
        wifi_strength = -ap_info.rssi;
    }
#else
    wifi_strength = 0;
#endif

    MONITOR_UPDATE(wifi_strength);
}

void jolt_hw_monitor_get_lock_status(hardware_monitor_t *monitor) {
    MONITOR_UPDATE(!(vault->valid));
}

/* Creates all the hardware_monitor mutex's and sets their updater functions */
static void jolt_hw_monitor_init() {
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

