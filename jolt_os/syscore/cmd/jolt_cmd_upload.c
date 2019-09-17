#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "lang/lang.h"
#include "stdio.h"
#include "syscore/bg.h"
#include "syscore/cli.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"
#include "syscore/ymodem/ymodem_common.h"

static const char TAG[] = "cmd_upload";

/**
 * @brief Callback for loading screen.
 */
static void jolt_cmd_upload_cb( lv_obj_t *bar, lv_event_t event )
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

/**
 * @brief Runs in background; actually performs the ymodem upload.
 */
static int jolt_cmd_upload_ymodem_task( jolt_bg_job_t *job )
{
    char *provided_name = jolt_bg_get_param( job );
    int return_code     = -1;
    char *orig_fullpath = NULL, *orig_fn = NULL;
    FILE *ffd                   = NULL;
    int rec_res                 = -1;
    int32_t max_fsize           = jolt_fs_free();
    jolt_gui_obj_t *loading_scr = NULL;

    /* Create loading screen */
    if( NULL == ( loading_scr = jolt_gui_scr_loadingbar_create( gettext( JOLT_TEXT_UPLOAD ) ) ) ) EXIT( -1 );
    jolt_gui_scr_set_event_cb( loading_scr, jolt_cmd_upload_cb );
    int8_t *progress = jolt_gui_scr_loadingbar_autoupdate( loading_scr );
    jolt_gui_scr_loadingbar_update( loading_scr, NULL, gettext( JOLT_TEXT_CONNECTING ), 0 );

    /* Parse filenames */
    if( NULL != provided_name && '\0' != provided_name[0] ) {
        orig_fullpath = jolt_fs_parse( provided_name, NULL );
        orig_fn       = provided_name;
    }
    else {
        jolt_fs_parse_buf( &orig_fullpath, &orig_fn );
    }
    if( NULL == orig_fullpath ) EXIT( -1 );

    /* Pre-run cleanup */
    if( jolt_fs_exists( JOLT_FS_TMP_FN ) ) remove( JOLT_FS_TMP_FN );

    /* Open tmp file */
    if( NULL == ( ffd = fopen( JOLT_FS_TMP_FN, "wb" ) ) ) { EXIT_PRINT( -2, "Error opening tmp for receive." ); }

    /* Perform Transfer */
    rec_res = ymodem_receive( ffd, max_fsize, orig_fn, progress );

    SAFE_CLOSE( ffd );

    if( rec_res <= 0 ) {
        /* Failure */
        char buf[64];
        ESP_LOGE( TAG, "Transfer complete, Error=%d", rec_res );
        snprintf( buf, sizeof( buf ), "%s=%d", gettext( JOLT_TEXT_ERROR ), rec_res );
        jolt_gui_scr_loadingbar_update( loading_scr, NULL, buf, -1 );
        /* Allow screen to be deleted via back button */
        jolt_gui_scr_set_event_cb( loading_scr, jolt_gui_event_del );
        EXIT( -5 );
    }

    JOLT_GUI_OBJ_DEL_SAFE( loading_scr );
    printf( "\"%s\" Transfer complete, Size=%d Bytes\n", orig_fn, rec_res );

    ESP_LOGI( TAG, "Renaming file" );
    if( jolt_fs_exists( orig_fullpath ) && 0 != remove( orig_fullpath ) ) {
        EXIT_PRINT( -3, "Unable to delete existing file. Transfer failed." );
    }
    if( 0 != rename( JOLT_FS_TMP_FN, orig_fullpath ) ) { EXIT_PRINT( -4, "Unable to rename file. Transfer failed." ); }

    jolt_fn_home_refresh( orig_fn );
    jolt_apply_patch( orig_fn );

    EXIT( 0 );

exit:
    jolt_resume_logging();
    jolt_cli_resume();
    SAFE_CLOSE( ffd );
    if( jolt_fs_exists( JOLT_FS_TMP_FN ) ) remove( JOLT_FS_TMP_FN );
    SAFE_FREE( orig_fullpath );
    JOLT_GUI_OBJ_DEL_SAFE( loading_scr );
    jolt_cli_return( return_code );

    return 0;
}

static void jolt_cmd_upload_vault_failure_cb( void *dummy )
{
    send_CA();
    jolt_cli_return( -1 );
    jolt_resume_logging();
    jolt_cli_resume();
}

/**
 * @brief Creating the loading screen and dispatch ymodem_task to bg.
 */
static void jolt_cmd_upload_vault_success_cb( void *name )
{
    if( ESP_OK != ( jolt_bg_create( jolt_cmd_upload_ymodem_task, name, NULL ) ) ) goto exit;
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
static void jolt_cmd_upload_confirmation_cb( jolt_gui_obj_t *obj, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        // Require PIN
        char *name_param;
        name_param = jolt_gui_scr_get_active_param( obj );
        jolt_gui_scr_del( obj );
        jolt_settings_vault_set( jolt_cmd_upload_vault_failure_cb, jolt_cmd_upload_vault_success_cb, name_param );
    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( obj );
        jolt_cli_return( -1 );
        jolt_resume_logging();
        jolt_cli_resume();
    }
}

int jolt_cmd_upload( int argc, char **argv )
{
    jolt_gui_obj_t *scr = NULL;
    char body[256 + JOLT_FS_MAX_FILENAME_LEN];
    const char *name = argv[1];
    char *name_param = argv[1];

    jolt_suspend_logging();

    if( !console_check_range_argc( argc, 1, 2 ) ) goto exit;
    if( 1 == argc ) {
        name       = gettext( JOLT_TEXT_UNKNOWN );
        name_param = NULL;
    }
    snprintf( body, sizeof( body ) - 1, gettext( JOLT_TEXT_TRANSFER_TO_JOLT ), name );

    if( NULL == ( scr = jolt_gui_scr_text_create( gettext( JOLT_TEXT_UPLOAD ), body ) ) ) goto exit;
    jolt_gui_scr_set_active_param( scr, name_param );
    jolt_gui_scr_set_event_cb( scr, jolt_cmd_upload_confirmation_cb );

    return JOLT_CLI_NON_BLOCKING;

exit:
    JOLT_GUI_OBJ_DEL_SAFE( scr );
    send_CA();
    jolt_resume_logging();
    jolt_cli_resume();
    return -1;
}
