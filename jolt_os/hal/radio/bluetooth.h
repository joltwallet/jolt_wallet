#ifndef JOLT_BLUETOOTH_H___
#define JOLT_BLUETOOTH_H___

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED
extern FILE *ble_stdin;
extern FILE *ble_stdout;
extern FILE *ble_stderr;

#endif

/* General start/stop */
esp_err_t jolt_bluetooth_start();
esp_err_t jolt_bluetooth_stop();

/* Advertising */
esp_err_t jolt_bluetooth_adv_all_start();
esp_err_t jolt_bluetooth_adv_wht_start();
esp_err_t jolt_bluetooth_adv_stop();

#endif
