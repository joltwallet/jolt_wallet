#ifndef __JOLT_HAL_HW_MONITORS_H__
#define __JOLT_HAL_HW_MONITORS_H__

#include <stdint.h>

typedef struct hardware_monitor_t {
    uint8_t val;
    void (*update)(struct hardware_monitor_t *); // function to call to update val
} hardware_monitor_t;

void jolt_hw_monitor_task();

void jolt_hw_monitor_get_lock_status(hardware_monitor_t *monitor);

void jolt_hw_monitor_update();

#ifndef CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS
    #define CONFIG_JOLT_HW_MONITOR_UPDATE_PERIOD_MS 2000
#endif

#endif
