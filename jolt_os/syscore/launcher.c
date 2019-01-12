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
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "vault.h"
#include "syscore/filesystem.h"
#include "jolt_lib.h"
#include "syscore/launcher.h"

static const char* TAG = "syscore_launcher";

static struct {
    jelfLoaderContext_t *ctx; /* Jelf Loader Context */
    lv_obj_t *scr;            /* LVGL Screen Object. Null if the user has exited */
    int argc;
    char **argv;
    char name[65];            /* App Name */
    bool loading;             /* True until app actually begins executing */
} app_cache = { 0 };

static lv_res_t launch_app_exit(lv_obj_t *btn);
static lv_res_t launch_app_from_store(lv_obj_t *btn);

static void launch_app_cache_clear(){
    if( NULL != app_cache.scr) {
        ESP_LOGE(TAG, "Cannot clear app_cache with a valid screen");
    }
    else {
        if(app_cache.ctx != NULL){
            jelfLoaderFree(app_cache.ctx);
        }
        app_cache.ctx = NULL;
        app_cache.name[0] = '\0';
        app_cache.argc = 0;
        app_cache.argv = NULL;
    }
}

/* Launches app specified without ".elf" suffix. i.e. "app"
 * Launches the app's function with same name as func.
 *
 * If the app was the last app launched (or currently launched app), it will
 * use the last cached instance (unless vault has been invalidated and 
 * matches derivation path)
 */
int launch_file(const char *fn_basename, int app_argc, char** app_argv){
    int return_code = -1;
    lv_obj_t *preloading_scr = NULL;
    LOADER_FD_T program = NULL;
	char exec_fn[128] = SPIFFS_BASE_PATH;

    if( true == app_cache.loading ){
        ESP_LOGW(TAG, "Cannot process additional app requests until "
                "previous app request resolves.");
        return_code = -2;
        goto exit;
    }
    app_cache.loading = true;

    if( NULL != app_cache.ctx ) {
        ESP_LOGI(TAG, "An app is already cached. Checking...");
        if( 0 == strcmp(app_cache.name, fn_basename) ) {
            if( app_argc > 0 ) {
                /* CLI command; Skip all the loading, goto execution */
                goto exec;
            }
            else if( launch_in_app() ){
                ESP_LOGW(TAG, "App is already launched");
                return_code = -4;
                goto exit;
            }
            else {
                ESP_LOGI(TAG, "Launching GUI for cached app");
                goto exec;
            }
        }
        else if( launch_in_app() ){
            ESP_LOGW(TAG, "Cannot launch a different app while in app");
            return_code = -3;
            goto exit;
        }
        else {
            /* Different app, clear cache and proceed */
            launch_app_cache_clear();
        }
    }

    /* Parse Exec Filename.
     * Takes something like "app" into  "/spiffs/app.jelf" */
	strcat(exec_fn, "/");
	strncat(exec_fn, fn_basename, sizeof(exec_fn)-strlen(exec_fn)-1-4);
    strcat(exec_fn, ".jelf");

    if( jolt_fs_exists(exec_fn) != 1 ){
        ESP_LOGE(TAG, "Executable doesn't exist\n");
        return -2;
    }

    preloading_scr = jolt_gui_scr_preloading_create(fn_basename, gettext(JOLT_TEXT_PRELOAD_LAUNCHING));

    program = fopen(exec_fn, "rb");

    #if CONFIG_JELFLOADER_PROFILER_EN
    jelfLoaderProfilerReset();
    #endif
    uint32_t jelfLoader_time = esp_timer_get_time();

    ESP_LOGI(TAG, "jelfLoader; Initializing");
    /* fn_basename is passed in for signature checking */
    if( NULL == (app_cache.ctx = jelfLoaderInit(program, fn_basename, &jelf_loader_env)) ) {
        return_code = -4;
        goto exit;
    }

    ESP_LOGI(TAG, "elfLoader; Loading Sections");
    if( NULL == jelfLoaderLoad(app_cache.ctx) ) {
        return_code = -5;
        goto exit;
    }

    ESP_LOGI(TAG, "elfLoader; Relocating");
    if( NULL == jelfLoaderRelocate(app_cache.ctx) ) {
        return_code = -6;
        goto exit;
    }

    jelfLoader_time = esp_timer_get_time() - jelfLoader_time;
    ESP_LOGI(TAG, "Application Loaded in %d uS.", jelfLoader_time);
    #if CONFIG_JELFLOADER_PROFILER_EN
    jelfLoaderProfilerPrint();
    #endif

    fclose(program);

    lv_obj_del(preloading_scr);
    preloading_scr = NULL;

    strcpy(app_cache.name, fn_basename);

exec:
    /* Verify Signature */
    if(!jelfLoaderSigCheck(app_cache.ctx)) {
        ESP_LOGE(TAG, "Bad Signature");
        char hash[HEX_512] = { 0 };
        sodium_bin2hex(hash, sizeof(hash), jelfLoaderGetHash(app_cache.ctx), 64);
        ESP_LOGE(TAG, "App Hash: %s", hash);
        goto exit;
    }
    /* Prepare vault for app launching. vault_set() creates the PIN entry screen */
    // maybe move these out of cache
    app_cache.argc = app_argc;
    app_cache.argv = app_argv;

    ESP_LOGI( TAG, "Derivation Purpose: 0x%x. Coin Type: 0x%x",
            app_cache.ctx->coin_purpose,
            app_cache.ctx->coin_path );
    ESP_LOGI(TAG, "The following BIP32 Key is %d char long:%s.",
            strlen(app_cache.ctx->bip32_key),
            app_cache.ctx->bip32_key);
    vault_set(app_cache.ctx->coin_purpose, 
            app_cache.ctx->coin_path,
            app_cache.ctx->bip32_key, 
            NULL, launch_app_from_store);

    app_cache.loading = false;
    return 0;

exit:
    launch_app_cache_clear();
    if( NULL != program) {
       fclose(program);
    }
    if( NULL != preloading_scr ) {
        lv_obj_del(preloading_scr);
    }

    app_cache.loading = false;
    return return_code;
}

static lv_res_t launch_app_from_store(lv_obj_t *dummy) {
    ESP_LOGI(TAG, "Launching App");
    app_cache.scr = (lv_obj_t *)jelfLoaderRun(app_cache.ctx,
            app_cache.argc, app_cache.argv);
    app_cache.loading = false;
    jolt_gui_scr_set_back_action(app_cache.scr, launch_app_exit);
    return LV_RES_OK;
}

static lv_res_t launch_app_exit(lv_obj_t *btn) {
    /* Mapped to back button of app's main menu */
    if( NULL != app_cache.scr ) {
        ESP_LOGI(TAG, "Deleting App Screen.");
        lv_obj_del(app_cache.scr);
        app_cache.scr = NULL;
        return LV_RES_INV;
    }
    else{
        return LV_RES_OK;
    }
}

bool launch_in_app(){
    if( NULL != app_cache.scr ) {
        return true;
    }
    else {
        return false;
    }
}

char *launch_get_name() {
    return app_cache.name;
}
