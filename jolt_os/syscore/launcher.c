/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "jelfloader.h"

#include "syscore/cli.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "vault.h"
#include "syscore/filesystem.h"
#include "jolt_lib.h"
#include "syscore/launcher.h"

static const char* TAG = "syscore_launcher";

static struct {
    jelfLoaderContext_t *ctx; /**< Jelf Loader Context */
    lv_obj_t *scr;            /**< LVGL Screen Object. */
    int argc;
    char **argv;
    char name[JOLT_FS_MAX_FILENAME_BUF_LEN]; /**< App Name */
    uint8_t ref_ctr;          /**< Number of references to currently running app code. Can only unload app if this is 0. */
    bool loading;             /* True until app actually begins executing */
} app_cache = { 0 };

static void launch_app_cb(lv_obj_t *btn, lv_event_t event);

static void launch_vault_fail_cb(void *dummy);
static void launch_app_from_store(void *dummy);

static void launch_app_cache_clear(){
    if( NULL != app_cache.scr && 0 != app_cache.ref_ctr) {
        ESP_LOGE(TAG, "Cannot clear app_cache with a valid screen");
    }
    else {
        if(app_cache.ctx != NULL){
            jelfLoaderFree(app_cache.ctx);
        }
        app_cache.ctx = NULL;
        sodium_memzero(app_cache.name, sizeof(app_cache.name));
        app_cache.argc = 0;
        app_cache.argv = NULL;
    }
}

/* Launches app specified without ".jelf" suffix. i.e. "app"
 * Launches the app's function with same name as func.
 *
 * If the app was the last app launched (or currently launched app), it will
 * use the last cached instance (unless vault has been invalidated and 
 * matches derivation path)
 */
int launch_file(const char *fn_basename, int app_argc, char** app_argv, const char *passphrase){
    int return_code = JOLT_LAUNCHER_ERR_UNKNOWN_FAIL;
    lv_obj_t *preloading_scr = NULL;
    LOADER_FD_T program = NULL;
    char *exec_fn = NULL;

    if( true == app_cache.loading ){
        ESP_LOGW(TAG, "Cannot process additional app requests until "
                "previous app request resolves.");
        return_code = -2;
        goto exit;
    }
    app_cache.loading = true;

    /* Check filename length */
    if( NULL == (exec_fn = jolt_fs_parse(fn_basename, "jelf")) ) {
        ESP_LOGE(TAG, "Too long of an app name");
        jolt_gui_scr_text_create(fn_basename, gettext(JOLT_TEXT_LAUNCH_INVALID) );
        EXIT(JOLT_LAUNCHER_ERR_UNKNOWN_FAIL);
    }
    ESP_LOGD(TAG, "App fullpath \"%s\"", exec_fn);

    if( NULL != app_cache.ctx ) {
        ESP_LOGI(TAG, "An app is already cached. Checking...");
        if( 0 == strcmp(app_cache.name, fn_basename) ) {
            if( app_argc > 0 ) {
                /* CLI command; Skip all the loading, goto execution */
                goto exec;
            }
            else if( launch_in_app() ){
                ESP_LOGW(TAG, "App is already launched");
                EXIT(JOLT_LAUNCHER_ERR_ALREADY_LAUNCHED);
            }
            else {
                ESP_LOGI(TAG, "Launching GUI for cached app");
                goto exec;
            }
        }
        else if( launch_in_app() ){
            ESP_LOGW(TAG, "Cannot launch a different app while in app");
            EXIT(JOLT_LAUNCHER_ERR_IN_APP);
        }
        else {
            /* Different app, clear cache and proceed */
            launch_app_cache_clear();
        }
    }

    if( !jolt_fs_exists(exec_fn) ){
        ESP_LOGE(TAG, "Executable doesn't exist\n");
        EXIT(JOLT_LAUNCHER_ERR_DOESNT_EXIST);
    }

    preloading_scr = jolt_gui_scr_preloading_create(fn_basename, gettext(JOLT_TEXT_PRELOAD_LAUNCHING));

    program = fopen(exec_fn, "rb");

    jelfLoaderProfilerReset();
    uint32_t jelfLoader_time = esp_timer_get_time();

    ESP_LOGI(TAG, "jelfLoader; Initializing");
    /* fn_basename is passed in for signature checking */
    switch( jelfLoaderInit(&app_cache.ctx, program, fn_basename, &jelf_loader_env) ) {
        case JELF_LOADER_OK:
            ESP_LOGI(TAG, "Context initialization OK.");
            break;
        case JELF_LOADER_VERSION_APP:
            jolt_gui_scr_text_create(fn_basename, gettext(JOLT_TEXT_LAUNCH_APP_OUT_OF_DATE) );
            EXIT(JOLT_LAUNCHER_ERR_APP_OUT_OF_DATE);
        case JELF_LOADER_VERSION_JOLTOS:
            jolt_gui_scr_text_create(fn_basename, gettext(JOLT_TEXT_LAUNCH_JOLTOS_OUT_OF_DATE) );
            EXIT(JOLT_LAUNCHER_ERR_OS_OUT_OF_DATE);
        default:
            jolt_gui_scr_text_create(fn_basename, gettext(JOLT_TEXT_LAUNCH_INVALID) );
            EXIT(JOLT_LAUNCHER_ERR_UNKNOWN_FAIL);
    }

    ESP_LOGI(TAG, "elfLoader; Loading Sections");
    switch( jelfLoaderLoad(app_cache.ctx) ) {
        case JELF_LOADER_OK:
            break;
        default:
            jolt_gui_scr_text_create(fn_basename, gettext(JOLT_TEXT_LAUNCH_INVALID) );
            EXIT(JOLT_LAUNCHER_ERR_LOADING_SECTION);
    }

    ESP_LOGI(TAG, "elfLoader; Relocating");
    switch( jelfLoaderRelocate(app_cache.ctx) ) {
        case JELF_LOADER_OK:
            break;
        default:
            jolt_gui_scr_text_create(fn_basename, gettext(JOLT_TEXT_LAUNCH_INVALID) );
            EXIT(JOLT_LAUNCHER_ERR_RELOCATING);
    }

    jelfLoader_time = esp_timer_get_time() - jelfLoader_time;
    ESP_LOGI(TAG, "Application Loaded in %d uS.", jelfLoader_time);
    jelfLoaderProfilerPrint();

    fclose(program);

    jolt_gui_scr_del(preloading_scr);
    preloading_scr = NULL;

    strlcpy(app_cache.name, fn_basename, sizeof(app_cache.name));

exec:
    /* Verify Signature */
    if(!jelfLoaderSigCheck(app_cache.ctx)) {
        ESP_LOGE(TAG, "Invalid App Signature");
        jolt_gui_scr_text_create(fn_basename, gettext(JOLT_TEXT_LAUNCH_INVALID) );
        EXIT(JOLT_LAUNCHER_ERR_SIGNATURE);
    }
    /* Prepare vault for app launching. vault_set() creates the PIN entry screen */
    app_cache.argc = app_argc;
    app_cache.argv = app_argv;

    ESP_LOGI( TAG, "Derivation Purpose: 0x%x. Coin Type: 0x%x",
            app_cache.ctx->coin_purpose,
            app_cache.ctx->coin_path );
    ESP_LOGI(TAG, "The following BIP32 Key is %d char long:%s.",
            strlen(app_cache.ctx->bip32_key),
            app_cache.ctx->bip32_key);
    launch_inc_ref_ctr();
    vault_set(app_cache.ctx->coin_purpose, 
            app_cache.ctx->coin_path,
            app_cache.ctx->bip32_key, 
            "",
            launch_vault_fail_cb, launch_app_from_store, NULL);

    app_cache.loading = false;
    return JOLT_LAUNCHER_ERR_OK;

exit:
    launch_app_cache_clear();
    SAFE_CLOSE(program);
    JOLT_GUI_OBJ_DEL_SAFE(preloading_scr);
    SAFE_FREE(exec_fn);
    app_cache.loading = false;
    return return_code;
}

/**
 * @brief Callback for when vault_set fails from app launch.
 */
static void launch_vault_fail_cb(void *dummy) {
    ESP_LOGE(TAG, "Launching app aborted");
    if(app_cache.argc > 0) {
        jolt_cli_return(-1);
    }
    else{
        launch_dec_ref_ctr();
    }
}

static void launch_app_from_store(void *dummy) {
    /* Runs in the BG task */
    int res;
    ESP_LOGI(TAG, "Launching App");

    res = jelfLoaderRun(app_cache.ctx, app_cache.argc, app_cache.argv);

    if( 0 == app_cache.argc ){
        /* Application returns a LVGL screen */
        app_cache.scr = (lv_obj_t*)res;
        if( NULL != app_cache.scr ) {
            jolt_gui_scr_set_event_cb(app_cache.scr, launch_app_cb);
        }
    }
    else{
        /* Application returns a return code */
        assert( res !=  JOLT_CLI_NON_BLOCKING );
        jolt_cli_return( res );
    }
}

/**
 * @brief Event handler for app's primary screen
 */
static void launch_app_cb(lv_obj_t *btn, lv_event_t event) {
    if( LV_EVENT_CANCEL == event ) {
        if( NULL != app_cache.scr ) {
            ESP_LOGI(TAG, "Deleting App Screen.");
            lv_obj_del(app_cache.scr);
            app_cache.scr = NULL;
        }
        ESP_LOGD(TAG, "%s decrementing counter.", __func__);
        launch_dec_ref_ctr();
    }
}

bool launch_in_app() {
    return app_cache.ref_ctr > 0;
}

char *launch_get_name() {
    return app_cache.name;
}

void launch_inc_ref_ctr() {
    if(app_cache.ref_ctr == UINT8_MAX){
        ESP_LOGE(TAG, "Cannot increment reference counter.");
    }
    else {
        app_cache.ref_ctr++;
        ESP_LOGD(TAG, "Reference Counter incremented to %d.", app_cache.ref_ctr);
    }
}

void launch_dec_ref_ctr() {
    if(app_cache.ref_ctr > 0) {
        app_cache.ref_ctr--;
        ESP_LOGD(TAG, "Reference Counter decremented to %d.", app_cache.ref_ctr);
    }
    else {
        ESP_LOGE(TAG, "Reference Counter is 0; cannot decrement");
    }
}
