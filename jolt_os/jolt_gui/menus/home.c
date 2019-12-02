//#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

#include "home.h"
#include "esp_log.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/settings/settings.h"
#include "jolt_gui/test_screens.h"
#include "jolt_helpers.h"
#include "sdkconfig.h"
#include "syscore/bg.h"
#include "syscore/filesystem.h"
#include "syscore/launcher.h"

/**********************
 *  STATIC PROTOTYPES
 **********************/
static int launch_app_task( jolt_bg_job_t *job );

/**********************
 *  STATIC VARIABLES
 **********************/
static const char TAG[]           = "menu_home";
static lv_obj_t *main_menu        = NULL;
static volatile bool need_refresh = false;

/**
 * @brief Task to launch the application from the BG task.
 *
 * Loading an app Takes on the order of 100~500mS, so we don't want to load it
 * within the GUI event loop.
 *
 */
static int launch_app_task( jolt_bg_job_t *job )
{
    char *fn_c = jolt_bg_get_param( job );
    // Launching App from homescreen uses an empty passphrase.
    jolt_launch_file( fn_c, 0, NULL, EMPTY_STR );
    return 0;  // Don't repeat.
}

/* App launching is spawned in the background task because it's a bit intense.
 * Also launch_file is a blocking function*/
static void launch_file_proxy( lv_obj_t *btn, lv_event_t event )
{
    if( LV_EVENT_SHORT_CLICKED == event ) {
        const char *fn;
        fn = lv_list_get_btn_text( btn );
        ESP_LOGI( TAG, "Launching %s", fn );
        jolt_bg_create( launch_app_task, (void *)fn, NULL );
    }
}

static void refresh()
{
    if( NULL != main_menu ) {
        jolt_gui_obj_del( main_menu );
        main_menu = NULL;
    }
    jolt_gui_menu_home_create();
    need_refresh = false;
}

static void home_cb( lv_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.focused == event && need_refresh ) { refresh(); }
    // don't allow the home screen to be deleted.
}

void jolt_gui_menu_home_create()
{
    // Find and Register all user apps
    char **fns     = NULL;
    uint16_t n_fns = jolt_fs_get_all_jelf_fns( &fns );

    main_menu = RESTART_IF_NULL( jolt_gui_scr_menu_create( gettext( JOLT_TEXT_MAIN_MENU_TITLE ) ) );

    for( uint16_t i = 0; i < n_fns; i++ ) {
        ESP_LOGD( TAG, "Registering App \"%s\" into the GUI", fns[i] );
        jolt_gui_scr_menu_add( main_menu, NULL, fns[i], launch_file_proxy );
    }
    jolt_free_char_array( fns, n_fns );

    jolt_gui_scr_menu_add( main_menu, NULL, gettext( JOLT_TEXT_SETTINGS ), menu_settings_create );
#if JOLT_GUI_TEST_MENU
    {
        lv_obj_t *elem;
        elem = jolt_gui_scr_menu_add( main_menu, NULL, "JSON", jolt_gui_test_json_create );
        jolt_gui_scr_menu_add_info( elem, "123" );
    }
    jolt_gui_scr_menu_add( main_menu, NULL, "BigNum", jolt_gui_test_bignum_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "QR", jolt_gui_test_qrcode_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "Loading", jolt_gui_test_loading_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "Autoloading", jolt_gui_test_autoloading_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "Preloading", jolt_gui_test_preloading_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "Number", jolt_gui_test_number_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "Battery", jolt_gui_test_battery_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "Alphabet and Scrolling Menu Option", jolt_gui_test_alphabet_create );
    jolt_gui_scr_menu_add( main_menu, NULL, "Https", jolt_gui_test_https_create );
#endif
    jolt_gui_scr_set_event_cb( main_menu, home_cb );
}

/* Refreshes the home menu.
 * Use cases: call after downloading an app. */
void jolt_gui_menu_home_refresh()
{
    if( jolt_gui_scr_get( NULL ) == main_menu ) {
        // If already the focused screen, directly cause a refresh.
        refresh();
    }
    else {
        // Queue up a refresh so the screen will refresh when focused.
        need_refresh = true;
    }
}
