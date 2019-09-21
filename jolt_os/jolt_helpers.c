/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

//#define LOG_LOCAL_LEVEL 4

#include "jolt_helpers.h"
#include <esp_system.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bipmnemonic.h"
#include "bootloader_random.h"
#include "esp_err.h"
#include "esp_hdiffz.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_vfs_dev.h"
#include "hal/storage/storage.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/menus/home.h"
#include "jolttypes.h"
#include "sdkconfig.h"
#include "sodium.h"
#include "syscore/filesystem.h"
#include "syscore/https.h"
#include "vault.h"

#if CONFIG_JOLT_STORE_ATAES132A
    #include "aes132_cmd.h"
#endif

static const char *TAG                    = "jolt_helpers";
const char NULL_TERM                      = '\0';
const char *EMPTY_STR                     = "";
const char JOLT_OS_DERIVATION_BIP32_KEY[] = "JOLT_OS";
const char *JOLT_OS_DERIVATION_PASSPHRASE = "";

void jolt_get_random( uint8_t *buf, uint8_t n_bytes )
{
    if( NULL == buf || 0 == n_bytes ) return;

#if CONFIG_JOLT_STORE_ATAES132A
    {
        bool locked;
        if( aes132_check_configlock( &locked ) ) { ESP_LOGE( TAG, "Unable to check ATAES132A lock status." ); }
        if( locked ) {
            ESP_LOGD( TAG, "Getting %d bytes from ATAES132A RNG Source", n_bytes );
            uint8_t res = aes132_rand( buf, n_bytes );
            if( ESP_OK != res ) {
                ESP_LOGE( TAG, "aes132_rand returned nonzero value %d", res );
                esp_restart();
            }
        }
        else {
            ESP_LOGD( TAG, "ATAES132A is not locked; not a secure RNG source." );
        }
    }
#endif

    /* ESP32 Strong RNG source */
    {
        ESP_LOGD( TAG, "Getting %d bytes from ESP32 RNG Source", n_bytes );
        CONFIDENTIAL uint8_t rand_buffer[4];
        for( uint8_t i = 0, j = 0; i < n_bytes; i += 4 ) {
            *(uint32_t *)rand_buffer = esp_random();
            for( uint8_t k = 0; k < 4; k++ ) {
                buf[j] ^= rand_buffer[k];
                j++;
                if( j == n_bytes ) break;
            }
        }
        sodium_memzero( rand_buffer, sizeof( rand_buffer ) );
    }
}

void shuffle_arr( uint8_t *arr, int arr_len )
{
    uint8_t tmp;
    for( int i = arr_len - 1; i > 0; i-- ) {
        uint32_t idx;
        jolt_get_random( (uint8_t *)&idx, sizeof( idx ) );
        idx      = idx % ( i + 1 );
        tmp      = arr[idx];
        arr[idx] = arr[i];
        arr[i]   = tmp;
    }
    sodium_memzero( &tmp, sizeof( uint8_t ) );
}

char **jolt_malloc_char_array( int n ) { return (char **)calloc( n, sizeof( char * ) ); }

void jolt_free_char_array( char **arr, int n )
{
    for( uint32_t i = 0; i < n; i++ ) { free( arr[i] ); }
    free( arr );
}

bool jolt_strcmp_suffix( const char *str, const char *suffix )
{
    if( NULL == str || NULL == suffix ) return false;

    uint32_t str_len    = strlen( str );
    uint32_t suffix_len = strlen( suffix );

    if( suffix_len > str_len ) { return false; }

    if( 0 == strcmp( str + str_len - suffix_len, suffix ) ) { return true; }

    return false;
}

void jolt_fn_home_refresh( const char *str )
{
    if( !jolt_strcmp_suffix( str, ".jelf" ) ) { return; }
    jolt_gui_menu_home_refresh();
}

void jolt_settings_vault_set( vault_cb_t fail_cb, vault_cb_t success_cb, void *param )
{
    vault_set( JOLT_OS_DERIVATION_PURPOSE, JOLT_OS_DERIVATION_PATH, JOLT_OS_DERIVATION_BIP32_KEY,
               JOLT_OS_DERIVATION_PASSPHRASE, fail_cb, success_cb, param );
}

void jolt_apply_patch( const char *filename )
{
    FILE *f_diff = NULL, *f_jelf = NULL, *f_tmp = NULL;
    char *path_diff = NULL;
    if( !jolt_strcmp_suffix( filename, ".patch" ) ) goto exit;

    /* Ensure its a full path */
    path_diff = jolt_fs_parse( filename, NULL );
    if( NULL == path_diff ) goto exit;

    /* Open DIFF file */
    f_diff = fopen( path_diff, "rb" );
    if( NULL == f_diff ) goto exit;

    if( 0 == jolt_strcmp_suffix( path_diff, "joltos.patch" ) ) {
        /* Firmware Update */
        // TODO: GUI stuff
        esp_hdiffz_ota_file( f_diff );
        SAFE_CLOSE( f_diff );
        remove( path_diff );
        esp_restart();
    }
    else {
        /* File Update */
        esp_err_t err;

        jolt_fs_strip_ext( path_diff );
        strcat( path_diff, ".jelf" );  // NOTE: only works because "jelf" is shorter than "patch"

        f_jelf = fopen( path_diff, "rb" );
        if( f_jelf == NULL ) goto exit;

        // Will output to /store/tmp
        f_tmp = fopen( JOLT_FS_TMP_FN, "wb" );
        if( f_tmp == NULL ) goto exit;

        err = esp_hdiffz_patch_file( f_jelf, f_tmp, f_diff );
        if( ESP_OK != err ) goto exit;

        SAFE_CLOSE( f_jelf );
        SAFE_CLOSE( f_tmp );
        SAFE_CLOSE( f_diff );

        /* Replace the app with the patched version */
        err = jolt_fs_mv( JOLT_FS_TMP_FN, path_diff );
        if( ESP_OK != err ) goto exit;
    }

exit:
    SAFE_FREE( path_diff );
    SAFE_CLOSE( f_diff );
    SAFE_CLOSE( f_jelf );
    SAFE_CLOSE( f_tmp );
    return;
}

static SemaphoreHandle_t suspend_logging_sem = NULL;
void jolt_suspend_logging()
{
    esp_log_level_set( "*", ESP_LOG_NONE );

    if( NULL == suspend_logging_sem ) {
        suspend_logging_sem = xSemaphoreCreateCounting( 200, 0 );  // arbitrarily high value.
        if( NULL == suspend_logging_sem ) { esp_restart(); }
    }

    xSemaphoreGive( suspend_logging_sem );
}

void jolt_resume_logging()
{
    if( NULL == suspend_logging_sem || !xSemaphoreTake( suspend_logging_sem, 0 ) ) {
        assert( 0 );
        esp_restart();
    }
    if( 0 == uxSemaphoreGetCount( suspend_logging_sem ) ) {
#if !CONFIG_JOLT_CONSOLE_OVERRIDE_LOGGING
        esp_log_level_set( "*", CONFIG_LOG_DEFAULT_LEVEL );
        esp_log_level_set( "wifi", ESP_LOG_NONE );
#endif
    }
}

int jolt_bytes_to_hstr( char *buf, size_t size, size_t bytes, uint8_t precision )
{
    const char *suffix[] = {"B", "KB", "MB", "GB", "TB"};
    uint8_t n_suffix        = sizeof( suffix ) / sizeof( suffix[0] );

    uint8_t i;
    double dbytes = bytes;

    for( i = 0; dbytes >= 1024 && ( i < n_suffix ); i++, dbytes /= 1024 )
        ;

    return snprintf( buf, size, "%0.*lf %s", precision, dbytes, suffix[i] );
}

int jolt_copy_until_space( char *buf, size_t size, const char *input )
{
    int i;

    /* Input Validation */
    if( NULL == input ) return -1;
    if( 0 == size ) buf = NULL;

    for( i = 0; i < INT_MAX && *input != '\0' && *input != ' '; input++, i++ ) {
        if( buf && size > 1 ) { /* To leave room for NULL-terminator */
            *buf++ = *input;
            size--;
        }
    }

    if( buf && size > 0 ) *buf = '\0';

    return i;
}

#if CONFIG_HEAP_POISONING_LIGHT || CONFIG_HEAP_POISONING_COMPREHENSIVE
    #define HEAP_OVERHEAD ( 4 + 12 )
#else
    #define HEAP_OVERHEAD ( 4 )  // TODO verify this value
#endif

void **jolt_consume_mem( size_t remain, size_t chunksize )
{
#if JOLT_GUI_TEST_MENU
    if( chunksize < sizeof( void * ) ) return NULL;
    void **consumed = NULL, **prev = NULL, **current;
    size_t available;
    available = heap_caps_get_free_size( MALLOC_CAP_8BIT );

    while( available > remain + chunksize + HEAP_OVERHEAD ) {
        current = calloc( 1, chunksize );
        if( NULL == prev )
            consumed = current;
        else
            *prev = current;
        if( NULL == current ) break;
        prev      = current;
        available = heap_caps_get_free_size( MALLOC_CAP_8BIT );
    }

    return consumed;
#else
    return NULL;
#endif
}

void jolt_consume_mem_free( void **consumed )
{
    if( NULL == consumed ) return;

    void *next;
    while( NULL != *consumed ) {
        next = *consumed;
        free( consumed );
        consumed = next;
    }
}
