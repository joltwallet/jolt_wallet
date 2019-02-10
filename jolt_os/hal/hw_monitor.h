#ifndef __JOLT_HAL_HW_MONITORS_H__
#define __JOLT_HAL_HW_MONITORS_H__

#include <stdint.h>
#include "sdkconfig.h"
#include "jolt_gui/jolt_gui_statusbar.h"

typedef struct hardware_monitor_t {
    int8_t val;
    void (*update)(struct hardware_monitor_t *); // function to call to update val
} hardware_monitor_t;

void jolt_hw_monitor_task();

void jolt_hw_monitor_get_lock_status(hardware_monitor_t *monitor);

void jolt_hw_monitor_update();

typedef enum jolt_bluetooth_level_t {
    JOLT_BLUETOOTH_LEVEL_OFF = 0,
    JOLT_BLUETOOTH_LEVEL_ON,
    JOLT_BLUETOOTH_LEVEL_CONN,
} jolt_bluetooth_level_t;

/* Holds HW Status */
extern hardware_monitor_t statusbar_indicators[JOLT_GUI_STATUSBAR_INDEX_NUM];

#ifndef CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS
    #define CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS 2000
#endif

#endif
