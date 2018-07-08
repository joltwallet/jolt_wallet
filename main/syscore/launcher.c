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

#define MAX_FILE_SIZE (1048576 - 0x2000)
#define SPIFFS_BASE_PATH "/spiffs"

static int launcher_run(int argc, char** argv) {
    int return_code;

    if( !console_check_equal_argc(argc, 2) ) {
        return_code = 1;
        goto exit;
    }

	char fn[128]=SPIFFS_BASE_PATH;
	strcat(fn, "/");
	strncat(fn, argv[1], sizeof(fn)-strlen(fn)-1);

    printf( "Reading %s\n", fn);
    FILE *f = fopen(fn, "rb");
    uint32_t *fake_f = f;
    for(int i=0; i<26; i++){
        printf("%d: 0x%x\n", i, fake_f[i]);
        //printf("sizeof(FILE): %d", sizeof(FILE));
    }

    int r = elfLoader(f, &env, "app_main", 0x10);
    fclose(f);

    return_code = 0;

exit:
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
