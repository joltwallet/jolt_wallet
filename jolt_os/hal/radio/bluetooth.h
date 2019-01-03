#ifndef JOLT_BLUETOOTH_H___
#define JOLT_BLUETOOTH_H___

#include "sdkconfig.h"

void jolt_bluetooth_setup();

#if CONFIG_BT_ENABLED
extern FILE *ble_stdin;
extern FILE *ble_stdout;
extern FILE *ble_stderr;
#endif


#endif
