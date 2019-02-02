#include "sdkconfig.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_pm.h"

int jolt_cmd_pmtop(int argc, char** argv) {
#if CONFIG_PM_ENABLE && CONFIG_PM_PROFILING
    esp_pm_dump_locks(stdout);
    return 0;
#else
    printf("JoltOS was not compiled with CONFIG_PM_PROFILING enabled\n");
    return -1;
#endif
}
