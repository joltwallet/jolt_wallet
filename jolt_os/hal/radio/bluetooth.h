#ifndef JOLT_BLUETOOTH_H___
#define JOLT_BLUETOOTH_H___

void jolt_bluetooth_setup();

void esp_vfs_dev_ble_spp_register();

extern FILE *ble_stdin;
extern FILE *ble_stdout;
extern FILE *ble_stderr;


#endif
