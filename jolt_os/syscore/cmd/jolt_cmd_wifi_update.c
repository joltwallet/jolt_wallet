#include "freertos/FreeRTOS.h"
#include "stdio.h"
#include "esp_system.h"
#include "syscore/set_wifi_credentials.h"
#include "syscore/cli_helpers.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/cli.h"

int jolt_cmd_wifi_update(int argc, char** argv) {
    int return_code = 0;

    if( !console_check_range_argc(argc, 2, 3) ) {
        return_code = 1;
        goto exit;
    }

    char *ssid = argv[1];
    char *pass = NULL;
    if( 3 == argc ) {
        pass = argv[2];
    }
    set_wifi_credentials( ssid, pass );

    return_code = JOLT_CLI_NON_BLOCKING;

exit:
    return return_code;
}

