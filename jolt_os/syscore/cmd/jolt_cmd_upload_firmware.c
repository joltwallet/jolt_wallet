/**
 *
 */

//#define LOG_LOCAL_LEVEL 4

#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/bg.h"
#include "syscore/cli.h"
#include "syscore/cli_helpers.h"
#include "syscore/ota.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"
#include "syscore/ymodem/ymodem_common.h"
#include "esp_hdiffz.h"

static const char TAG[] = "cmd_upload_firmware";

/* Use these enums to update the subtitle upon percentage change */
enum {
    FIRMWARE_UPGRADE_STATE_NONE,
    FIRMWARE_UPGRADE_STATE_TRANSFERRING,
};
typedef uint8_t firmware_upgrade_state_t;

typedef struct {
    firmware_upgrade_state_t state;
    bool is_patch;
} firmware_upgrade_ctx_t;
static firmware_upgrade_ctx_t ctx;


static void jolt_cmd_upload_firmware_cb( lv_obj_t *bar, lv_event_t event )
{
    if( jolt_gui_event.apply == event ) {
        // No action
    }
    else if( jolt_gui_event.value_changed == event ) {
        int8_t *progress;
        progress = jolt_gui_scr_loadingbar_progress_get( bar );

        if(*progress > 0 && ctx.state != FIRMWARE_UPGRADE_STATE_NONE) {
            /* Callbacks to update subtitle depending on progress.
             * (Subtitles that cannot be updated via inline blocking code) */
            switch(ctx.state) {
                case FIRMWARE_UPGRADE_STATE_TRANSFERRING:
                    jolt_gui_scr_loadingbar_update( bar, NULL, gettext( JOLT_TEXT_TRANSFERRING ), -1 ); 
                    break;
                default:
                    // Reaches here in an undefined state, abort for safety.
                    abort();
            }
            ctx.state = FIRMWARE_UPGRADE_STATE_NONE;
        }
    }
    else if( jolt_gui_event.cancel == event ) {
        // TODO: Tell bg jolt_cmd_upload_ymodem_task to abort, clean up, etc.
    }
}

static int jolt_cmd_upload_firmware_ymodem_task( jolt_bg_job_t *job )
{
    char buf[64];  /* For joltgui loading screen messages */
    int return_code             = -1;
    jolt_gui_obj_t *loading_scr = NULL;
    int8_t *progress            = NULL;

    /* Create loading screen */
    if( NULL == ( loading_scr = jolt_gui_scr_loadingbar_create( gettext( JOLT_TEXT_UPLOAD ) ) ) ) EXIT( -1 );
    jolt_gui_scr_set_event_cb( loading_scr, jolt_cmd_upload_firmware_cb );
    progress = jolt_gui_scr_loadingbar_autoupdate( loading_scr );
    jolt_gui_scr_loadingbar_update( loading_scr, NULL, gettext( JOLT_TEXT_CONNECTING ), 0 );

    ctx.state = FIRMWARE_UPGRADE_STATE_TRANSFERRING;

    if(ctx.is_patch) {
        esp_err_t err;
        FILE *ffd = NULL;
        int32_t max_fsize = jolt_fs_free();
        int rec_res;

        /* Pre-run cleanup */
        if( jolt_fs_exists( JOLT_FS_TMP_FN ) ) remove( JOLT_FS_TMP_FN );

        /* Open tmp file */
        if( NULL == ( ffd = fopen( JOLT_FS_TMP_FN, "wb" ) ) ) { EXIT_PRINT( -2, "Error opening tmp for receive." ); }

        /* Perform Transfer */
        rec_res = ymodem_receive( ffd, max_fsize, NULL, progress );
        if( rec_res <= 0 ) {
            /* Failure */
            ESP_LOGE( TAG, "Transfer incomplete, Error=%d", rec_res );
            snprintf( buf, sizeof( buf ), "%s=%d", gettext( JOLT_TEXT_ERROR ), rec_res );
            jolt_gui_scr_loadingbar_update( loading_scr, NULL, buf, -1 );
            EXIT( -5 );
        }

        /* Serial communication is now done, so we can safely resume logging */
        jolt_resume_logging();

        ESP_LOGI(TAG, "Upload successful");

        /* Close and re-open tmp file with read capabilities*/
        SAFE_CLOSE(ffd);
        if( NULL == ( ffd = fopen( JOLT_FS_TMP_FN, "rb" ) ) ) { EXIT_PRINT( -8, "Error opening tmp for reading." ); }

        jolt_gui_scr_loadingbar_update( loading_scr, NULL, gettext(JOLT_TEXT_INSTALLING), 0 );
        vTaskDelay(50); // small delay to allow screen to update. TODO: mitigate gui task starvation

        /* Apply the patch */
        err = esp_hdiffz_ota_file_progress(ffd, progress);

        /* Clean up resources prior to error check */
        SAFE_CLOSE(ffd);
        remove( JOLT_FS_TMP_FN );

        if(ESP_OK != err) {
            /* Failure */
            ESP_LOGE(TAG, "Failed to apply patch");
            snprintf( buf, sizeof( buf ), "%s=%d", gettext( JOLT_TEXT_ERROR ), -9 );
            jolt_gui_scr_loadingbar_update( loading_scr, NULL, buf, -1 );
            EXIT( -9 );
        }
    }
    else{
        /* Begin transfer/update */
        if( ESP_OK != jolt_ota_ymodem( progress ) ) {
            ESP_LOGE( TAG, "OTA Failure" );
            snprintf( buf, sizeof( buf ), "%s=%d", gettext( JOLT_TEXT_ERROR ), -10 );
            jolt_gui_scr_loadingbar_update( loading_scr, NULL, buf, -1 );
            EXIT( -10 );
        }
        // TODO: Verify integrity of next ota partition to tell user if
        // firmware transfer was successful/unsuccessful
    }

    ESP_LOGI( TAG, "OTA Success; rebooting..." );
    jolt_gui_scr_loadingbar_update( loading_scr, NULL, gettext( JOLT_TEXT_REBOOTING ), -1 );
    vTaskDelay(pdMS_TO_TICKS(1000));  /* Pause slightly so user can see message */

    esp_restart();

exit:
    /* Reaching here means that an error MUST have occurred */
    jolt_resume_logging();
    jolt_cli_resume();
    if(loading_scr) {
        /* Allow screen to be deleted via back button */
        jolt_gui_scr_set_event_cb( loading_scr, jolt_gui_event_del );
    }
    jolt_cli_return( return_code );
    return 0;  // Dont repeat this task
}

static void jolt_cmd_upload_firmware_vault_failure_cb( void *dummy )
{
    send_CA();
    jolt_cli_return( -1 );
    jolt_resume_logging();
    jolt_cli_resume();
}

/**
 * @brief Creating the loading screen and dispatch ymodem_task to bg.
 */
static void jolt_cmd_upload_firmware_vault_success_cb( void *name )
{
    if( ESP_OK != ( jolt_bg_create( jolt_cmd_upload_firmware_ymodem_task, name, NULL ) ) ) goto exit;
    return;

exit:
    send_CA();
    jolt_cli_return( -1 );
    jolt_resume_logging();
    jolt_cli_resume();
}

/**
 * @brief Callback for "Accept Transfer" confirmation screen.
 */
static void jolt_cmd_upload_firmware_confirmation_cb( jolt_gui_obj_t *obj, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_scr_del( obj );
        jolt_settings_vault_set( jolt_cmd_upload_firmware_vault_failure_cb, jolt_cmd_upload_firmware_vault_success_cb,
                                 NULL );
    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( obj );
        jolt_cli_return( -1 );
        jolt_resume_logging();
        jolt_cli_resume();
    }
}

int jolt_cmd_upload_firmware( int argc, char **argv )
{
    jolt_gui_obj_t *scr = NULL;
    char body[256];

    ctx.state = FIRMWARE_UPGRADE_STATE_NONE;
    ctx.is_patch = false;

    /* parse cli args */
    for(uint8_t i=1; i < argc; i++) {
        if(0 == strcmp(argv[i], "--patch")) {
            ESP_LOGD(TAG, "Expecting Patch Data");
            ctx.is_patch = true;
        }
        else {
            ESP_LOGE(TAG, "Invalid arg %s", argv[i]);
            goto exit;
        }
    }

    jolt_suspend_logging();

    snprintf( body, sizeof( body ), gettext( JOLT_TEXT_TRANSFER_TO_JOLT ), "JoltOS" );

    if( NULL == ( scr = jolt_gui_scr_text_create( gettext( JOLT_TEXT_UPLOAD ), body ) ) ) goto exit;
    jolt_gui_scr_set_event_cb( scr, jolt_cmd_upload_firmware_confirmation_cb );

    return JOLT_CLI_NON_BLOCKING;

exit:
    JOLT_GUI_OBJ_DEL_SAFE( scr );
    send_CA();
    jolt_resume_logging();
    jolt_cli_resume();
    return -1;
}
