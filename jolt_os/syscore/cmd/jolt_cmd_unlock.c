//#define LOG_LOCAL_LEVEL 4

#include "sdkconfig.h"

#if JOLT_GUI_TEST_MENU

#include "stdio.h"
#include "esp_log.h"
#include "jolt_helpers.h"
#include "cJSON.h"

//static const char TAG[] = "jolt_cmd_unlock";

int jolt_cmd_unlock(int argc, char **argv) {
    jolt_settings_vault_set(NULL, NULL, NULL);
    return 0;
}

#endif
