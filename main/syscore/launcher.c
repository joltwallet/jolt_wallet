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

#include "esp_spiffs.h"

#include "elfloader.h"
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
    int return_code = -1;
    ELFLoaderContext_t *ctx = NULL;
    uint32_t *data = NULL;
    char *program_buf = NULL;

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

    {
        // Reading the whole App to memory is 1~2 orders of magnitude faster
        // than POSIX ops on file pointers
        ESP_LOGI(TAG, "Reading in file");
        FILE *f = NULL;
        f = fopen(exec_fn, "rb");
        fseek(f, 0, SEEK_END);
        size_t fsize = ftell(f);
        fseek(f, 0, SEEK_SET);
        if( NULL == (program_buf = malloc(fsize)) ) {
            ESP_LOGE(TAG, "Couldn't allocate space for program buffer.");
            fclose(f);
            goto exit;
        }
        fread(program_buf, fsize, 1, f);
        fclose(f);
    }

    ESP_LOGI(TAG, "elfLoader; Initializing");
    if( NULL == (ctx = elfLoaderInit(program_buf, &env)) ) {
        goto exit;
    }
    // todo: Load and Relocate in the background while the user enter's their pin
    ESP_LOGI(TAG, "elfLoader; Loading Sections");
    if( NULL == elfLoaderLoad(ctx) ) {
        goto exit;
    }
    ESP_LOGI(TAG, "elfLoader; Relocating");
    if( NULL == elfLoaderRelocate(ctx) ) {
        goto exit;
    }
    ESP_LOGI(TAG, "elfLoader; Setting Entrypoint");
    if( 0 != elfLoaderSetFunc(ctx, func) ) {
        goto exit;
    }
    {
        size_t data_len;
        uint32_t purpose, coin;
        char bip32_key[32];
#define PATH_BYTE_LEN 8
        data = elfLoaderLoadSectionByName(ctx, ".coin.path", &data_len);
        if( NULL==data ) {
            ESP_LOGE(TAG, "Couldn't allocate for .coin.path");
            goto exit;
        }
        if( data_len <= (PATH_BYTE_LEN + 1) || 
                data_len>=(PATH_BYTE_LEN+sizeof(bip32_key))) {
            ESP_LOGE(TAG, "BIP32_Key not provided in ELF file.");
            goto exit;
        }
        purpose = *data;
        coin = *(data+1);
        size_t bip32_key_len = data_len-PATH_BYTE_LEN; // Not including null terminator
#undef PATH_BYTE_LEN
        strncpy(bip32_key, (char *)(data+2), bip32_key_len);
        bip32_key[bip32_key_len+1] = '\0';
        ESP_LOGI(TAG,"Derivation Purpose: 0x%x. Coin Type: 0x%x",
                purpose, coin);
        ESP_LOGI(TAG, "The following BIP32 Key is %d char long:%s.", bip32_key_len, bip32_key);
        if( !vault_set(purpose, coin, bip32_key) ) {
            ESP_LOGI(TAG, "User aborted app launch at PIN screen");
            goto exit;
        }
    }

    free(program_buf);
    program_buf = NULL;

    ESP_LOGI(TAG, "Launching App");
    return_code = elfLoaderRun(ctx, app_argc, app_argv);
    ESP_LOGI(TAG, "App closed");

    return_code = 0;

exit:
    if( NULL != ctx ) {
        elfLoaderFree(ctx);
    }
    if( NULL != program_buf ){
        free(program_buf);
    }
    if( NULL != data ){
        free(data);
    }

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
