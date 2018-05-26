#ifndef __JOLT_HELPERS_H__
#define __JOLT_HELPERS_H__

#include "u8g2.h"
#include "menu8g2.h"

void nvs_log_err(esp_err_t err);
void factory_reset();
bool store_mnemonic_reboot(menu8g2_t *menu, char *mnemonic);

#endif
