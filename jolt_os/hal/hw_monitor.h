/**
 * @file hw_monitor.h
 * @brief Polls various systems' status
 * @author Brian Pugh
 */

#ifndef __JOLT_HAL_HW_MONITORS_H__
#define __JOLT_HAL_HW_MONITORS_H__

#include <stdint.h>
#include "jolt_gui/jolt_gui_statusbar.h"
#include "sdkconfig.h"

/**
 * @brief Generic hardware monitor structure
 */
typedef struct hardware_monitor_t {
    int8_t val;                                     /**< System's reported value */
    void ( *update )( struct hardware_monitor_t* ); /**< function to periodically call to update val */
} hardware_monitor_t;

/**
 * @brief Indices into statusbar_indicators array
 */
enum {
    JOLT_HW_MONITOR_INDEX_BATTERY = 0,
    JOLT_HW_MONITOR_INDEX_WIFI,
    JOLT_HW_MONITOR_INDEX_BLUETOOTH,
    JOLT_HW_MONITOR_INDEX_LOCK,
    JOLT_HW_MONITOR_INDEX_NUM,
};

/**
 * @brief FreeRTOS task that periodically calls all hw_monitors' update functinos.
 */
void jolt_hw_monitor_task();

/**
 * @brief Bluetooth connection GUI states.
 */
typedef enum jolt_bluetooth_level_t {
    JOLT_BLUETOOTH_LEVEL_OFF = 0, /**< Bluetooth is off */
    JOLT_BLUETOOTH_LEVEL_ON,      /**< Bluetooth is on, but not connected */
    JOLT_BLUETOOTH_LEVEL_CONN,    /**< Bluetooth is on and connected */
} jolt_bluetooth_level_t;

/**
 * @brief Holds all hw_monitors
 */
extern hardware_monitor_t statusbar_indicators[JOLT_HW_MONITOR_INDEX_NUM];

#ifndef CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS
    #define CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS 2000
#endif

#endif
