#include "stdio.h"
#include "string.h"
#include "esp_system.h"
#include "syscore/console_helpers.h"
#include "hal/storage/storage.h"

int jolt_cmd_jolt_cast_update(int argc, char** argv) {
    int return_code;
    char buf[100];

    // Check if number of inputs is correct
    if( !console_check_equal_argc(argc, 2) ) {
        return_code = 1;
        goto exit;
    }

    storage_set_str(argv[1], "user", "jc_uri");

    // Confirm Inputs
    snprintf(buf, sizeof(buf), "Update jolt_cast server domain to:\n%s", argv[1]);

    // todo: require use confirmation on all input
#if 0
    if( !menu_confirm_action(menu, buf) ) {
        return_code = -1;
        goto exit;
    }
    snprintf(buf, sizeof(buf), "Update jolt_cast server path to:\n%s", argv[2]);
    if( !menu_confirm_action(menu, buf) ) {
        return_code = -1;
        goto exit;
    }
    snprintf(buf, sizeof(buf), "Update jolt_cast server port to:\n%d", port);
    if( !menu_confirm_action(menu, buf) ) {
        return_code = -1;
        goto exit;
    }
#endif

    esp_restart();

    exit:
        return return_code;
}
