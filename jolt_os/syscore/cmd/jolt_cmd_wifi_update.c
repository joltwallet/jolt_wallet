#include "freertos/FreeRTOS.h"
#include "stdio.h"
#include "esp_system.h"
#include "syscore/set_wifi_credentials.h"
#include "syscore/console_helpers.h"
#include "jolt_gui/jolt_gui.h"

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
    bool update_success = set_wifi_credentials( ssid, pass );

    if( update_success ) {
        printf("Wifi Settings Updated. Restarting...\n");
        jolt_gui_scr_text_create( "WiFi Update", "WiFi credentials updated. Rebooting system." );
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
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

