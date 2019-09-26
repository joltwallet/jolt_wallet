//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "jolt_gui_first_boot.h"
#include "bipmnemonic.h"
#include "esp_log.h"
#include "hal/storage/storage.h"
#include "jolt_gui.h"
#include "jolt_helpers.h"
#include "jolttypes.h"
#include "sodium.h"
#include "syscore/bg.h"

#define MNEMONIC_STRENGTH   256
#define SINGLE_WORD_BUF_LEN 15

typedef struct {
    CONFIDENTIAL uint256_t mnemonic_bin;
    CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
    CONFIDENTIAL uint256_t pin_hash;
} mnemonic_setup_t;

/********************
 * Static Variables *
 ********************/
static const char TAG[] = "first_boot";

/********************
 * Static Functions *
 ********************/
static void generate_mnemonic( mnemonic_setup_t *param );
static void mismatch_cb( lv_obj_t *btn, lv_event_t event );
static void screen_finish_create( lv_obj_t *verify_scr, lv_event_t event );
static void screen_pin_verify_create( lv_obj_t *digit_entry, lv_event_t event );
static void screen_pin_entry_create( lv_obj_t *btn, lv_event_t event );
static lv_obj_t *screen_pin_entry_create_( mnemonic_setup_t *param );
static void screen_mnemonic_create( lv_obj_t *btn, lv_event_t event );

static jolt_err_t get_nth_word( char buf[], size_t buf_len, const char *str, uint8_t n );

/**
 * @brief Get the nth word (0-indexed) from a space-separated string (mnemonic).
 *
 * Assumes a single space between words; no leading/trailing spaces.
 *
 * Will prepend the word with it's 1-indexed equivalent; e.g.
 *     20. cat
 *
 * Will return E_FAILURE if word is not found or if there is insufficient buffer.
 *
 * @param[out] buf Output buffer to store a copy of the word in.
 * @param[in] buf_len Size of output buffer.
 * @param[in] str NULL-terminated, space-delimited list of words
 * @param[in] n 0-indexed word to copy back. Must be <=24. For n==24, will return
 *              a "continue" string.
 */
static jolt_err_t get_nth_word( char buf[], size_t buf_len, const char *str, uint8_t n )
{
    uint16_t bytes_written;
    const char list_prefix[] = "%d. ";

    if( NULL == buf || NULL == str ) return E_FAILURE;

    if( n == 24 ) {
        strlcpy( buf, gettext( JOLT_TEXT_CONTINUE ), buf_len );
        return E_SUCCESS;
    }
    else if( n > 24 ) {
        ESP_LOGE( TAG, "IndexError: %d", n );
        return E_FAILURE;
    }

    // Copy over list prefix
    bytes_written = snprintf( buf, buf_len, list_prefix, n + 1 ); /* One-Index */
    if( bytes_written >= buf_len ) return E_FAILURE;
    buf_len -= bytes_written;
    buf += bytes_written;

    // Copy over the nth word
    for( uint8_t space_count = 0; *str != '\0'; str++ ) {
        if( *str == ' ' ) { space_count++; }
        else if( space_count == n ) {
            int word_len;
            word_len = jolt_copy_until_space( buf, buf_len, str );
            if( word_len < buf_len )
                return E_SUCCESS;
            else
                return E_FAILURE;
        }
    }
    return E_FAILURE;
}

/* Generates 256-bits of entropy into mnemonic_bin.
 * Computes mnemonic string mnemonic from mnemonic_bin. The string is
 * used for display/backup purposes only.*/
static void generate_mnemonic( mnemonic_setup_t *param )
{
    jolt_random( param->mnemonic_bin, MNEMONIC_STRENGTH / 8 );
    bm_bin_to_mnemonic( param->mnemonic, sizeof( param->mnemonic ), param->mnemonic_bin, MNEMONIC_STRENGTH );
    ESP_LOGI( TAG, "mnemonic: %s", param->mnemonic );
}

/* BG Task for stretching */
static int bg_stretch_task( jolt_bg_job_t *job )
{
    int8_t *progress;
    lv_obj_t *loading_scr;
    mnemonic_setup_t *param;

    param = jolt_bg_get_param( job );

    /* Create Loading Bar */
    RESTART_IF_NULL( loading_scr = jolt_gui_scr_loadingbar_create( gettext( JOLT_TEXT_SAVING ) ) );
    jolt_gui_scr_loadingbar_update( loading_scr, NULL, gettext( JOLT_TEXT_PROCESSING ), 0 );
    progress = jolt_gui_scr_loadingbar_autoupdate( loading_scr );

    /* Perform the lengthy stretching */
    storage_stretch_init();
    storage_stretch( param->pin_hash, progress );

    jolt_gui_scr_loadingbar_update( loading_scr, NULL, gettext( JOLT_TEXT_SAVING ), 100 );

    /* Saves setup information to storage */
    storage_set_mnemonic( param->mnemonic_bin, param->pin_hash );
    storage_set_pin_count( 0 ); /* Only does something if pin_count is setable */
    uint32_t pin_count = storage_get_pin_count();
    storage_set_pin_last( pin_count );

    sodium_memzero( param->pin_hash, sizeof( param->pin_hash ) );
    sodium_memzero( param->mnemonic_bin, sizeof( param->mnemonic_bin ) );
    sodium_memzero( param->mnemonic, sizeof( param->mnemonic ) );

    esp_restart();

    return 0;
}

static void mismatch_cb( lv_obj_t *btn, lv_event_t event )
{
    if( jolt_gui_event.short_clicked == event || jolt_gui_event.cancel == event ) {
        mnemonic_setup_t *param;
        param = jolt_gui_obj_get_param( btn );
        jolt_gui_scr_del( btn );
        screen_pin_entry_create_( param );
    }
}

/* Verifies the re-entered pin-hash
 *     Success: Creates the stretching screen.
 *     Failure: Creates the mismatch screen, sends user back to pin entry.*/
static void screen_finish_create( lv_obj_t *digit_entry, lv_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        mnemonic_setup_t *param;
        int res;
        CONFIDENTIAL uint256_t pin_hash_verify;

        /* Compute Verify Pin Hash */
        if( 0 != jolt_gui_obj_digit_entry_get_hash( digit_entry, pin_hash_verify ) ) {
            ESP_LOGE( TAG, "Failed to get PIN hash" );
            esp_restart();  // irrecoverable error
        }
        param = jolt_gui_obj_get_param( digit_entry );

        res = memcmp( param->pin_hash, pin_hash_verify, sizeof( pin_hash_verify ) );
        sodium_memzero( pin_hash_verify, sizeof( pin_hash_verify ) );

        /* Delete verify pin entry screen */
        jolt_gui_scr_del( digit_entry );

        /* Verify the pins match */
        if( 0 == res ) { ESP_ERROR_CHECK( jolt_bg_create( bg_stretch_task, param, NULL ) ); }
        else {
            sodium_memzero( param->pin_hash, sizeof( param->pin_hash ) );
            lv_obj_t *scr =
                    jolt_gui_scr_text_create( gettext( JOLT_TEXT_PIN_SETUP ), gettext( JOLT_TEXT_PIN_MISMATCH ) );
            RESTART_IF_NULL( scr );
            jolt_gui_scr_set_active_param( scr, param );
            jolt_gui_scr_set_event_cb( scr, mismatch_cb );
        }
    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( digit_entry );
    }
}

/* Saves the entered pinhash. Creates the pin verify screen.
 * Creates StartupScreen 4
 * */
static void screen_pin_verify_create( lv_obj_t *digit_entry, lv_event_t event )
{
    ESP_LOGV( TAG, "(%d) event: %s", __LINE__, jolt_gui_event_to_str( event ) );
    if( jolt_gui_event.short_clicked == event ) {
        mnemonic_setup_t *param;
        /* Get the hash for the first screen */
        param = jolt_gui_obj_get_param( digit_entry );
        ESP_LOGD( TAG, "Got param %p", param );
        if( 0 != jolt_gui_obj_digit_entry_get_hash( digit_entry, param->pin_hash ) ) {
            ESP_LOGE( TAG, "Failed to get PIN hash" );
            esp_restart();  // irrecoverable error
        }

        /* Delete first PIN entry screen */
        ESP_LOGD( TAG, "Performig digit entry delete" );
        jolt_gui_scr_del( digit_entry );

        /* Create Verify PIN screen */
        ESP_LOGD( TAG, "Creating verify screen" );
        lv_obj_t *scr = jolt_gui_scr_digit_entry_create( gettext( JOLT_TEXT_PIN_VERIFY ), CONFIG_JOLT_GUI_PIN_LEN,
                                                         JOLT_GUI_SCR_DIGIT_ENTRY_NO_DECIMAL );
        RESTART_IF_NULL( scr );
        jolt_gui_scr_set_active_param( scr, param );
        ESP_LOGD( TAG, "Setting CB" );
        jolt_gui_scr_set_event_cb( scr, &screen_finish_create );
    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( digit_entry );
    }
}

static lv_obj_t *screen_pin_entry_create_( mnemonic_setup_t *param )
{
    lv_obj_t *scr = jolt_gui_scr_digit_entry_create( gettext( JOLT_TEXT_PIN ), CONFIG_JOLT_GUI_PIN_LEN,
                                                     JOLT_GUI_SCR_DIGIT_ENTRY_NO_DECIMAL );
    RESTART_IF_NULL( scr );
    ESP_LOGD( TAG, "(%d) Created PIN entry screen at %p", __LINE__, scr );
    jolt_gui_scr_set_active_param( scr, param );
    jolt_gui_scr_set_event_cb( scr, &screen_pin_verify_create );
    return scr;
}

/* Prompts user for pin (first time).
 * Creates StartupScreen3
 */
static void screen_pin_entry_create( lv_obj_t *btn, lv_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        mnemonic_setup_t *param;
        param = jolt_gui_obj_get_param( btn );
        screen_pin_entry_create_( param );
    }
}

/**
 * Displays 24-word mnemonic.
 * Creates StartupScreen2
 */
static void screen_mnemonic_create( lv_obj_t *btn, lv_event_t event )
{
    ESP_LOGV( TAG, "(%d) event: %s", __LINE__, jolt_gui_event_to_str( event ) );
    if( jolt_gui_event.short_clicked == event ) {
        mnemonic_setup_t *param;
        param = jolt_gui_obj_get_param( btn );

        lv_obj_t *scr = jolt_gui_scr_menu_create( gettext( JOLT_TEXT_BACKUP_MNEMONIC ) );
        RESTART_IF_NULL( scr );
        for( uint8_t i = 0; i < 24; i++ ) {
            char buf[SINGLE_WORD_BUF_LEN] = {0};
            jolt_gui_obj_t *elem;

            get_nth_word( buf, sizeof( buf ), param->mnemonic, i );

#if JOLT_GUI_TEST_MENU
            /* so devs dont have to scroll down every time */
            elem = jolt_gui_scr_menu_add( scr, NULL, buf, screen_pin_entry_create );
#else
            elem = jolt_gui_scr_menu_add( scr, NULL, buf, NULL );
#endif
            RESTART_IF_NULL( elem );
        }
        RESTART_IF_NULL( jolt_gui_scr_menu_add( scr, NULL, gettext( JOLT_TEXT_CONTINUE ), screen_pin_entry_create ) );
        jolt_gui_scr_menu_set_param( scr, param );
    }
    else if( jolt_gui_event.cancel == event ) {
        /* Do Nothing - delete action handled by default menu action */
    }
}

/**
 * Called externally to begin the first-boot GUI.
 *
 * Creates StartupScreen1
 */
void jolt_gui_first_boot_create()
{
    mnemonic_setup_t *param;
    param = RESTART_IF_NULL( malloc( sizeof( mnemonic_setup_t ) ) );

    generate_mnemonic( param );

    lv_obj_t *scr =
            jolt_gui_scr_text_create( gettext( JOLT_TEXT_FIRST_STARTUP ), gettext( JOLT_TEXT_WELCOME_DIALOG_0 ) );
    RESTART_IF_NULL( scr );
    jolt_gui_scr_set_active_param( scr, param );
    jolt_gui_scr_set_event_cb( scr, &screen_mnemonic_create );
}

/* Prompts user for pin and performs the rest of setup for a given entropy */
void jolt_gui_restore_sequence( const uint256_t mnemonic )
{
    mnemonic_setup_t *param;
    param = RESTART_IF_NULL( malloc( sizeof( mnemonic_setup_t ) ) );

    memcpy( param->mnemonic_bin, mnemonic, 32 );

    lv_obj_t *scr =
            jolt_gui_scr_digit_entry_create( "PIN", CONFIG_JOLT_GUI_PIN_LEN, JOLT_GUI_SCR_DIGIT_ENTRY_NO_DECIMAL );
    RESTART_IF_NULL( scr );
    jolt_gui_scr_set_active_param( scr, param );
    jolt_gui_scr_set_event_cb( scr, &screen_pin_verify_create );
}
