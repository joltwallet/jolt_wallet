#include "hw_monitor.h"
#include <driver/adc.h>
#include "esp_bt.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "hal/radio/bluetooth.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "sdkconfig.h"
#include "vault.h"

hardware_monitor_t statusbar_indicators[JOLT_HW_MONITOR_INDEX_NUM];

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void jolt_hw_monitor_get_battery_level( hardware_monitor_t *monitor );
static void jolt_hw_monitor_get_bluetooth_level( hardware_monitor_t *monitor );
static void jolt_hw_monitor_get_wifi_level( hardware_monitor_t *monitor );
static void jolt_hw_monitor_update();

/**********************
 *  STATIC VARIABLES
 **********************/
static const char TAG[] = "hal/monitor";

/**********************
 *  DEFINE MACROS
 **********************/
#define MONITOR_UPDATE( x ) monitor->val = x;

/* Responds with a value between 0 and 100.
 * If a value above 100 is returned, the battery is charging.
 *
 * Cycles for demonstration purpose */
static void jolt_hw_monitor_get_battery_level( hardware_monitor_t *monitor )
{
    static uint16_t vals[CONFIG_JOLT_VBATT_AVG_WINDOW] = {0};  // store moving average
    static uint8_t index                               = 0;
    // todo: check charging gpio

    // Get a new reading
    index       = ( index + 1 ) % CONFIG_JOLT_VBATT_AVG_WINDOW;
    vals[index] = (uint16_t)adc1_get_raw( JOLT_ADC1_VBATT );
    vals[index] = 100;

    // compute average
    uint32_t cum = 0;
    uint8_t n    = 0;
    assert( CONFIG_JOLT_VBATT_AVG_WINDOW <= UINT8_MAX );
    for( uint8_t i = 0; i < CONFIG_JOLT_VBATT_AVG_WINDOW; i++ ) {
        if( vals[i] != 0 ) {
            cum += vals[i];
            n++;
        }
    }
    uint32_t avg = cum / n;

    /* Note: this calculation represents a linear voltage drop assumption. */
    uint8_t percentage =
            ( uint8_t )( 100 * ( avg - CONFIG_JOLT_VBATT_MIN ) / ( CONFIG_JOLT_VBATT_MAX - CONFIG_JOLT_VBATT_MIN ) );
    if( percentage > 100 ) { percentage = 100; }

    ESP_LOGD( TAG, "[vbatt] raw: %d   percent: %d\n", avg, percentage );

    MONITOR_UPDATE( percentage );
}

static void jolt_hw_monitor_get_bluetooth_level( hardware_monitor_t *monitor )
{
    /* Returns with the bluetooth strength level. 0 if no connected. */
    uint8_t level = JOLT_BLUETOOTH_LEVEL_OFF;

#if CONFIG_BT_ENABLED
    esp_bt_controller_status_t status;
    status = esp_bt_controller_get_status();
    if( ESP_BT_CONTROLLER_STATUS_ENABLED == status ) {
        if( jolt_bluetooth_is_connected() ) { level = JOLT_BLUETOOTH_LEVEL_CONN; }
        else {
            level = JOLT_BLUETOOTH_LEVEL_ON;
        }
    }
#endif
    MONITOR_UPDATE( level );
}

static void jolt_hw_monitor_get_wifi_level( hardware_monitor_t *monitor )
{
    /* Returns with the wifi strength level */
    uint8_t wifi_strength;
#if !CONFIG_NO_BLOBS
    wifi_mode_t mode;
    esp_err_t err = esp_wifi_get_mode( &mode );
    if( ESP_OK == err ) {
        wifi_ap_record_t ap_info;
        if( esp_wifi_sta_get_ap_info( &ap_info ) != ESP_OK ) { wifi_strength = 0; }
        else {
            wifi_strength = -ap_info.rssi;
        }
    }
    else {
        wifi_strength = -1;
    }
#else
    wifi_strength = -1;
#endif

    MONITOR_UPDATE( wifi_strength );
}

static void jolt_hw_monitor_get_lock_status( hardware_monitor_t *monitor ) { MONITOR_UPDATE( !vault_get_valid() ); }

/* Creates all the hardware_monitor mutex's and sets their updater functions */
void jolt_hw_monitor_init()
{
    ESP_LOGI( TAG, "Starting hardware monitor task" );
    statusbar_indicators[JOLT_HW_MONITOR_INDEX_BATTERY].update   = &jolt_hw_monitor_get_battery_level;
    statusbar_indicators[JOLT_HW_MONITOR_INDEX_WIFI].update      = &jolt_hw_monitor_get_wifi_level;
    statusbar_indicators[JOLT_HW_MONITOR_INDEX_BLUETOOTH].update = &jolt_hw_monitor_get_bluetooth_level;
    statusbar_indicators[JOLT_HW_MONITOR_INDEX_LOCK].update      = &jolt_hw_monitor_get_lock_status;

    const esp_timer_create_args_t timer_args = {.callback = &jolt_hw_monitor_update, .name = "hw_monitor"};

    esp_timer_handle_t statusbar_update_timer;
    ESP_ERROR_CHECK( esp_timer_create( &timer_args, &statusbar_update_timer ) );
    ESP_ERROR_CHECK(
            esp_timer_start_periodic( statusbar_update_timer, CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS * 1000 ) );
}

/* Iterates through all hardware_monitor's and call's their updater function */
static void jolt_hw_monitor_update()
{
    for( uint8_t i = 0; i < JOLT_HW_MONITOR_INDEX_NUM; i++ ) {
        statusbar_indicators[i].update( &statusbar_indicators[i] );
    }
}
