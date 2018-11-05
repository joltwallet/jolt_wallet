/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"
#include "esp_spiffs.h"
#include "elfloader.h"

#include "console.h"
#include "jolt_globals.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "jolt_lib.h"
#include "vault.h"
#include "filesystem.h"
#include "jolt_lib.h"

static const char* TAG = "syscore_launcher";

#ifdef CONFIG_ELFLOADER_POSIX
#define LOADER_FD_FREE fclose
#elif CONFIG_ELFLOADER_MEMORY_POINTER
#define LOADER_FD_FREE free
#endif

static lv_action_t launch_app_exit(lv_obj_t *btn);
static lv_action_t launch_app_from_store(lv_obj_t *btn);

static bool check_elf_valid(char *fn) {
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
    if( NULL != jolt_gui_store.app.ctx ) {
        ESP_LOGE(TAG, "An app is already running");
        return -1;
    }

    LOADER_FD_T program = NULL;

    // Parse Exec Filename
	char exec_fn[128] = SPIFFS_BASE_PATH;
	strcat(exec_fn, "/");
	strncat(exec_fn, fn_basename, sizeof(exec_fn)-strlen(exec_fn)-1-4);
    strcat(exec_fn, ".elf");

    // Parse Signature Filename
    char sig_fn[128] = SPIFFS_BASE_PATH;
	strcat(sig_fn, "/");
	strncat(sig_fn, fn_basename, sizeof(sig_fn)-strlen(sig_fn)-1-4);
    strcat(sig_fn, ".sig");

    // todo: Make sure both files exist
    if( check_file_exists(exec_fn) != 1 ){
        ESP_LOGE(TAG, "Executable doesn't exist\n");
        return -2;
    }

    // TODO: Verify File Signature Here
#if 0
    if( check_file_exists(sig_fn) != 1 ){
        ESP_LOGE(TAG, "Signature doesn't exist\n");
        return_code = 2;
        goto exit;
    }
#endif

    // Reading the whole App to memory is 1~2 orders of magnitude faster
    // than POSIX ops on file pointers
    ESP_LOGI(TAG, "Decompressing %s", exec_fn);
    if( NULL == (program = decompress_file(exec_fn)) ) {
        ESP_LOGE(TAG, "Error decompressing %s", exec_fn);
        return -3;
    }
    ESP_LOGI(TAG, "mem pointer: %p", program);
    ESP_LOGI(TAG, "first 4 bytes: 0x%08x", *(uint32_t *)program);

    ESP_LOGI(TAG, "elfLoader; Initializing");
    if( NULL == (jolt_gui_store.app.ctx = elfLoaderInit(program, &env)) ) {
        elfLoaderFree(jolt_gui_store.app.ctx);
        jolt_gui_store.app.ctx = NULL;
        LOADER_FD_FREE(program);
        return -4;
    }

    ESP_LOGI(TAG, "elfLoader; Loading Sections");
    if( NULL == elfLoaderLoad(jolt_gui_store.app.ctx) ) {
        elfLoaderFree(jolt_gui_store.app.ctx);
        jolt_gui_store.app.ctx = NULL;
        LOADER_FD_FREE(program);
        return -5;
    }
    ESP_LOGI(TAG, "elfLoader; Relocating");
    if( NULL == elfLoaderRelocate(jolt_gui_store.app.ctx) ) {
        elfLoaderFree(jolt_gui_store.app.ctx);
        jolt_gui_store.app.ctx = NULL;
        LOADER_FD_FREE(program);
        return -6;
    }
    ESP_LOGI(TAG, "elfLoader; Setting Entrypoint");
    if( 0 != elfLoaderSetFunc(jolt_gui_store.app.ctx, func) ) {
        elfLoaderFree(jolt_gui_store.app.ctx);
        jolt_gui_store.app.ctx = NULL;
        LOADER_FD_FREE(program);
        return -7;
    }
    {
        uint32_t *data = NULL;
        size_t data_len;
        uint32_t purpose, coin;
        char bip32_key[33];
#define PATH_BYTE_LEN 8 // 4 bytes for purpose, 4 bytes for
        data = elfLoaderLoadSectionByName(jolt_gui_store.app.ctx, ".coin.path", &data_len);
        if( NULL==data ) {
            ESP_LOGE(TAG, "Couldn't allocate for .coin.path");
            elfLoaderFree(jolt_gui_store.app.ctx);
            jolt_gui_store.app.ctx = NULL;
            LOADER_FD_FREE(program);
            return -8;
        }
        if( data_len <= (PATH_BYTE_LEN + 1) || 
                data_len>=(PATH_BYTE_LEN+sizeof(bip32_key))) {
            ESP_LOGE(TAG, "Valid BIP32_Key not provided in ELF file.");
            elfLoaderFree(jolt_gui_store.app.ctx);
            jolt_gui_store.app.ctx = NULL;
            LOADER_FD_FREE(program);
            return -9;
        }
        purpose = *data;
        coin = *(data+1);
        size_t bip32_key_len = data_len-PATH_BYTE_LEN; // Not including null terminator
        strncpy(bip32_key, &((char *)data)[PATH_BYTE_LEN], bip32_key_len);
        bip32_key[bip32_key_len] = '\0'; // Null terminate string
        ESP_LOGI(TAG,"Derivation Purpose: 0x%x. Coin Type: 0x%x",
                purpose, coin);
        ESP_LOGI(TAG, "The following BIP32 Key is %d char long:%s.",
                bip32_key_len, bip32_key);
#undef PATH_BYTE_LEN

        /* now that all runtime data has been extracted from the decompressed 
         * ELF file, we can free that memory */
        LOADER_FD_FREE(program); // Unload decompressed ELF

        /* Prepare vault for app launching. This creates the PIN entry screen */
        jolt_gui_store.app.argc = app_argc;
        jolt_gui_store.app.argv = app_argv;
        vault_set(purpose, coin, bip32_key, 
                launch_app_exit, launch_app_from_store);
    }
    return 0;
}

static lv_action_t launch_app_from_store(lv_obj_t *btn) {
    ESP_LOGI(TAG, "Launching App");
    jolt_gui_store.app.scr = (lv_obj_t *)elfLoaderRun(jolt_gui_store.app.ctx, jolt_gui_store.app.argc, jolt_gui_store.app.argv);
    jolt_gui_scr_set_back_action(jolt_gui_store.app.scr, launch_app_exit);
    return 0;
}

static lv_action_t launch_app_exit(lv_obj_t *btn) {
    /* Delete the app menu and free up the app memory */
    if( NULL != jolt_gui_store.app.scr ) {
        ESP_LOGI(TAG, "Deleting App Screen.");
        lv_obj_del(jolt_gui_store.app.scr);
        jolt_gui_store.app.scr = NULL;
    }
    if( NULL != jolt_gui_store.app.ctx ) {
        ESP_LOGI(TAG, "Exitting App");
        elfLoaderFree(jolt_gui_store.app.ctx);
        jolt_gui_store.app.ctx = NULL;
    }
    return 0;
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
