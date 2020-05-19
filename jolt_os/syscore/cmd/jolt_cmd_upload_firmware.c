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
static bool is_patch;


static void jolt_cmd_upload_firmware_cb( lv_obj_t *bar, lv_event_t event )
{
    if( jolt_gui_event.apply == event ) {
        jolt_gui_scr_loadingbar_update( bar, NULL, gettext( JOLT_TEXT_INSTALLING ), -1 );
    }
    else if( jolt_gui_event.value_changed == event ) {
        int8_t *progress;
        progress = jolt_gui_scr_loadingbar_progress_get( bar );
        if( *progress > 0 ) { jolt_gui_scr_loadingbar_update( bar, NULL, gettext( JOLT_TEXT_TRANSFERRING ), -1 ); }
    }
    else if( jolt_gui_event.cancel == event ) {
        // TODO: Tell bg jolt_cmd_upload_ymodem_task to abort, clean up, etc.
    }
}

static int jolt_cmd_upload_firmware_ymodem_task( jolt_bg_job_t *job )
{
    int return_code             = -1;
    jolt_gui_obj_t *loading_scr = NULL;
    int8_t *progress            = NULL;

    /* Create loading screen */
    if( NULL == ( loading_scr = jolt_gui_scr_loadingbar_create( gettext( JOLT_TEXT_UPLOAD ) ) ) ) EXIT( -1 );
    jolt_gui_scr_set_event_cb( loading_scr, jolt_cmd_upload_firmware_cb );
    progress = jolt_gui_scr_loadingbar_autoupdate( loading_scr );
    jolt_gui_scr_loadingbar_update( loading_scr, NULL, gettext( JOLT_TEXT_CONNECTING ), 0 );

    if(is_patch) {
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
            char buf[64];
            ESP_LOGE( TAG, "Transfer incomplete, Error=%d", rec_res );
            snprintf( buf, sizeof( buf ), "%s=%d", gettext( JOLT_TEXT_ERROR ), rec_res );
            jolt_gui_scr_loadingbar_update( loading_scr, NULL, buf, -1 );
            /* Allow screen to be deleted via back button */
            jolt_gui_scr_set_event_cb( loading_scr, jolt_gui_event_del );
            EXIT( -5 );
        }

        ESP_LOGI(TAG, "Upload successful");

        /* apply the patch */
        ESP_LOGI(TAG, "Applying patch...");
        err = esp_hdiffz_ota_file(ffd);
        SAFE_CLOSE(ffd);
        remove( JOLT_FS_TMP_FN );

        if(ESP_OK != err) {
            /* Failure */
            char buf[64];

            ESP_LOGE(TAG, "Failed to apply patch");
            snprintf( buf, sizeof( buf ), "%s=%d", gettext( JOLT_TEXT_ERROR ), err );
            jolt_gui_scr_loadingbar_update( loading_scr, NULL, buf, -1 );
            /* Allow screen to be deleted via back button */
            jolt_gui_scr_set_event_cb( loading_scr, jolt_gui_event_del );
            EXIT( -6 );
        }
    }
    else{
        /* Begin transfer/update */
        if( ESP_OK != jolt_ota_ymodem( progress ) ) {
            ESP_LOGE( TAG, "OTA Failure" );
            jolt_gui_obj_del( loading_scr );

            EXIT( 0 );
        }

    }

    ESP_LOGI( TAG, "OTA Success; rebooting..." );

    esp_restart();


exit:
    jolt_resume_logging();
    jolt_cli_resume();
    JOLT_GUI_OBJ_DEL_SAFE( loading_scr );
    jolt_cli_return( return_code );
    return 0;
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

    /* parse cli args */
    is_patch = false;
    for(uint8_t i=1; i < argc; i++) {
        if(0 == strcmp(argv[i], "--patch")) {
            is_patch = true;
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
