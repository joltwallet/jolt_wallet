/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "menu8g2.h"
#include <libwebsockets.h>
#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include "nano_lws.h"
#include "nano_parse.h"

#include "esp_spiffs.h"

#include "loader.h"
#include "../jolt_lib.h"

#include "filesystem.h"
#include "../globals.h"
#include "../console.h"
#include "../vault.h"
#include "../helpers.h"
#include "../gui/gui.h"
#include "../gui/loading.h"
#include "../gui/statusbar.h"
#include "../gui/confirmation.h"

#include "../console.h"

static const char* TAG = "syscore_launcher";


bool check_elf_valid(char *fn) {
    /* Checks ths signature file for a given basename fn*/
    // TODO implement
    return false;
}

int launch_file(const char *fn_basename, const char *func, int app_argc, char** app_argv){
    /* Launches app specified without ".elf" suffix
     * This is done so signature file can be checked easier
     *
     * Launches the app's function with same name as func
     */
    int return_code;

    // Parse Exec Filename
	char exec_fn[128]=SPIFFS_BASE_PATH;
	strcat(exec_fn, "/");
	strncat(exec_fn, fn_basename, sizeof(exec_fn)-strlen(exec_fn)-1-4);
    strcat(exec_fn, ".elf");

    // Parse Signature Filename
    char sig_fn[128]=SPIFFS_BASE_PATH;
	strcat(sig_fn, "/");
	strncat(sig_fn, fn_basename, sizeof(sig_fn)-strlen(sig_fn)-1-4);
    strcat(sig_fn, ".sig");

    // Make sure both files exist
    if( check_file_exists(exec_fn) != 1 ){
        ESP_LOGE(TAG, "Executable doesn't exist\n");
        return_code = -100; // TODO: Better return code
        goto exit;
    }

    // TODO: Verify File Signature Here
#if 0
    if( check_file_exists(sig_fn) != 1 ){
        ESP_LOGE(TAG, "Signature doesn't exist\n");
        return_code = 2;
        goto exit;
    }
#endif
    
    FILE *f = fopen(exec_fn, "rb");
    return_code = elfLoader(f, &env, func, app_argc, app_argv);
    fclose(f);

exit:
    return return_code;
}

static int launcher_run(int argc, char** argv) {
    /* Takes in 2 arguments (elf_fn, entry_point)
     * the elf suffix will be added to elf_fn.
     * if entry_point is not provided, defaults to app_main
     */
    int return_code;

    char entry_point[64] = "app_main";
    if(argc >= 3) {
        strlcpy(entry_point, argv[2], sizeof(entry_point));
    }

    int app_argc = argc - 3;
    char **app_argv = NULL;
    if( app_argc <= 0 ) {
        app_argc = 0;
    }
    else{
        app_argv = argv + 3;
    }

    return_code = launch_file(argv[1], entry_point, app_argc, app_argv);

    return return_code;
}

void console_syscore_launcher_register() {
    esp_console_cmd_t cmd;

    cmd = (esp_console_cmd_t) {
        .command = "run",
        .help = "launch elf file",
        .hint = NULL,
        .func = &launcher_run,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

}
