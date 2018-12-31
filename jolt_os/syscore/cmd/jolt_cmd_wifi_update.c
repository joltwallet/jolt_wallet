#include "stdio.h"
#include "esp_system.h"
#include "hal/radio/wifi.h"
#include "syscore/console_helpers.h"

int jolt_cmd_wifi_update(int argc, char** argv) {
    int return_code;

    if( !console_check_range_argc(argc, 2, 3) ) {
        return_code = 1;
        goto exit;
    }

    bool update_success;
    if( 3 == argc ) {
        update_success = set_wifi_credentials(argv[1], argv[2]);
    }
    else {
        update_success = set_wifi_credentials(argv[1], "");
    }

    if( update_success ) {
        printf("Wifi Settings Updated. Restarting...\n");
        return_code = 0;
        esp_restart();
    }
    else {
        printf("Error Updating WiFi Settings\n");
        return_code = 1;
        goto exit;
    }

    exit:
        return return_code;
}

