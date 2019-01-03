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
#include "jelfloader.h"

#include "console.h"
#include "jolt_globals.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "vault.h"
#include "filesystem.h"
#include "jolt_lib.h"

static const char* TAG = "syscore_launcher";


static lv_res_t launch_app_exit(lv_obj_t *btn);
static lv_res_t launch_app_from_store(lv_obj_t *btn);

int launch_file(const char *fn_basename, int app_argc, char** app_argv){
    /* Launches app specified without ".elf" suffix. i.e. "app"
     * Launches the app's function with same name as func
     */
    int return_code = -1;
    if( NULL != jolt_gui_store.app.ctx ) {
        ESP_LOGE(TAG, "An app is already running");
        return -1;
    }

    LOADER_FD_T program = NULL;

    /* Parse Exec Filename.
     * Takes something like "app" into  "/spiffs/app.jelf" */
	char exec_fn[128] = SPIFFS_BASE_PATH;
	strcat(exec_fn, "/");
	strncat(exec_fn, fn_basename, sizeof(exec_fn)-strlen(exec_fn)-1-4);
    strcat(exec_fn, ".jelf");

    if( jolt_fs_exists(exec_fn) != 1 ){
        ESP_LOGE(TAG, "Executable doesn't exist\n");
        return -2;
    }

    jolt_gui_sem_take();
    lv_obj_t *preloading_scr = jolt_gui_scr_preloading_create(
            fn_basename, "Launching...");
    jolt_gui_sem_give();

    program = fopen(exec_fn, "rb");

    #if CONFIG_JELFLOADER_PROFILER_EN
    jelfLoaderProfilerReset();
    uint64_t jelfLoader_time = esp_timer_get_time();
    #endif

    ESP_LOGI(TAG, "jelfLoader; Initializing");
    /* fn_basename is passed in for signature checking */
    if( NULL == (jolt_gui_store.app.ctx = jelfLoaderInit(program, fn_basename, &env)) ) {
        return_code = -4;
        goto err;
    }

    ESP_LOGI(TAG, "elfLoader; Loading Sections");
    if( NULL == jelfLoaderLoad(jolt_gui_store.app.ctx) ) {
        return_code = -5;
        goto err;
    }
    ESP_LOGI(TAG, "elfLoader; Relocating");
    if( NULL == jelfLoaderRelocate(jolt_gui_store.app.ctx) ) {
        return_code = -6;
        goto err;
    }

    #if CONFIG_JELFLOADER_PROFILER_EN
    jelfLoader_time = esp_timer_get_time() - jelfLoader_time;
    ESP_LOGI(TAG, "Application Loaded in %lld uS.", jelfLoader_time);
    jelfLoaderProfilerPrint();
    #endif

    fclose(program);

    jolt_gui_sem_take();
    lv_obj_del(preloading_scr);
    jolt_gui_sem_give();

    /* Prepare vault for app launching. vault_set() creates the PIN entry screen */
    jolt_gui_store.app.argc = app_argc;
    jolt_gui_store.app.argv = app_argv;

    ESP_LOGI( TAG, "Derivation Purpose: 0x%x. Coin Type: 0x%x",
            jolt_gui_store.app.ctx->coin_purpose,
            jolt_gui_store.app.ctx->coin_path );
    ESP_LOGI(TAG, "The following BIP32 Key is %d char long:%s.",
            strlen(jolt_gui_store.app.ctx->bip32_key),
            jolt_gui_store.app.ctx->bip32_key);
    vault_set(jolt_gui_store.app.ctx->coin_purpose, 
            jolt_gui_store.app.ctx->coin_path,
            jolt_gui_store.app.ctx->bip32_key, 
            launch_app_exit, launch_app_from_store);

    return 0;

err:
    if(jolt_gui_store.app.ctx != NULL){
        jelfLoaderFree(jolt_gui_store.app.ctx);
    }
    jolt_gui_store.app.ctx = NULL;
    if( NULL != program) {
       fclose(program);
    }
    jolt_gui_sem_take();
    lv_obj_del(preloading_scr);
    jolt_gui_sem_give();

    return return_code;
}

static lv_res_t launch_app_from_store(lv_obj_t *btn) {
    ESP_LOGI(TAG, "Launching App");
    jolt_gui_store.app.scr = (lv_obj_t *)jelfLoaderRun(jolt_gui_store.app.ctx,
            jolt_gui_store.app.argc, jolt_gui_store.app.argv);
    jolt_gui_scr_set_back_action(jolt_gui_store.app.scr, launch_app_exit);
    return LV_RES_OK;
}

static lv_res_t launch_app_exit(lv_obj_t *btn) {
    /* Delete the app menu and free up the app memory */
    if( NULL != jolt_gui_store.app.scr ) {
        ESP_LOGI(TAG, "Deleting App Screen.");
        lv_obj_del(jolt_gui_store.app.scr);
        jolt_gui_store.app.scr = NULL;
    }
    if( NULL != jolt_gui_store.app.ctx ) {
        ESP_LOGI(TAG, "Exitting App");
        jelfLoaderFree(jolt_gui_store.app.ctx);
        jolt_gui_store.app.ctx = NULL;
    }
    return LV_RES_INV;
}
