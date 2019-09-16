#include "sdkconfig.h"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include <driver/adc.h>
#include "cJSON.h"
#include "esp_log.h"
#include "hal/hw_monitor.h"
#include "jolt_gui/jolt_gui.h"
#include "jolt_gui/test_screens.h"
#include "jolt_helpers.h"
#include "json_config.h"
#include "syscore/filesystem.h"
#include "syscore/https.h"

static const char TAG[] = "test_screens";

void jolt_gui_test_json_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    cJSON *json = NULL;
    char *path  = NULL;
    if( jolt_gui_event.short_clicked == event ) {
        if( NULL == ( path = jolt_fs_parse( "test", "json" ) ) ) goto exit;

        json = jolt_json_read( path );

        if( NULL == json ) {
            ESP_LOGI( TAG, "No json file found; creating." );
            json = cJSON_CreateObject();
            EXIT_IF_NULL( cJSON_AddStringToObject( json, "testStringKey", "testStringVal" ) );
            EXIT_IF_NULL( cJSON_AddNumberToObject( json, "testIntKey", 123456789 ) );
            jolt_json_write( path, json );
        }

        /* Print stuff to stdout*/
        {
            cJSON *obj;
            obj = cJSON_GetObjectItemCaseSensitive( json, "testStringKey" );
            printf( "testStringKey: %s\n", cJSON_GetStringValue( obj ) );
        }
    }

exit:
    if( json ) jolt_json_del( json );
    SAFE_FREE( path );
}

static void jolt_gui_test_number_enter_cb( jolt_gui_obj_t *digit_entry, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        double d_val   = jolt_gui_obj_digit_entry_get_double( digit_entry );
        uint32_t i_val = jolt_gui_obj_digit_entry_get_int( digit_entry );
        ESP_LOGI( TAG, "Entry as Double: %f", d_val );
        ESP_LOGI( TAG, "Entry as Integer: %d", i_val );
        jolt_gui_scr_del( digit_entry );
    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( digit_entry );
    }
}

void jolt_gui_test_number_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr = jolt_gui_scr_digit_entry_create( "Number Test", 5, 2 );
        /* Set the digit left of the dp */
        jolt_gui_scr_digit_entry_set_pos( scr, 2 );
        jolt_gui_scr_set_event_cb( scr, jolt_gui_test_number_enter_cb );
    }
}

void jolt_gui_test_qrcode_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        const char data[] = "Meow";
        jolt_gui_scr_qr_create( "QR Test", "Meow", sizeof( data ) );
    }
}

void jolt_gui_test_preloading_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr;
        scr = jolt_gui_scr_preloading_create( "Preloading", "Status Message" );
        if( NULL == scr ) return;
        jolt_gui_scr_set_event_cb( scr, jolt_gui_event_del );
    }
}

static void test_loading_task( void *param )
{
    jolt_gui_obj_t *scr = (jolt_gui_obj_t *)param;
    for( uint8_t i = 0; i < 101; vTaskDelay( pdMS_TO_TICKS( 1000 ) ), i += 10 ) {
        if( i == 50 ) { jolt_gui_scr_loadingbar_update( scr, "Almost Done", "woof", i ); }
        else if( i > 50 ) {
            jolt_gui_scr_loadingbar_update( scr, NULL, "bark", i );
        }
        else {
            jolt_gui_scr_loadingbar_update( scr, NULL, "meow", i );
        }
    }
    lv_obj_del( scr );
    vTaskDelete( NULL );
}

void jolt_gui_test_loading_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr = jolt_gui_scr_loadingbar_create( "Loading Test" );
        if( NULL == scr ) { ESP_LOGE( TAG, "NULL Loading Screen" ); }
        xTaskCreate( test_loading_task, "TestLoading", 4096, (void *)scr, 10, NULL );
    }
}

static void test_autoloading_cb( lv_obj_t *bar, lv_event_t event )
{
    if( jolt_gui_event.apply == event ) {
        ESP_LOGD( TAG, "(%d; %s) autoloading complete!", __LINE__, __func__ );
        jolt_gui_obj_del( jolt_gui_scr_get( bar ) );
    }
    else if( jolt_gui_event.value_changed == event ) {
        char buf[30];
        int8_t *progress;
        progress = jolt_gui_scr_loadingbar_progress_get( bar );
        snprintf( buf, sizeof( buf ), "Progress: %d%%", *progress );
        ESP_LOGD( TAG, "(%d): %s", __LINE__, buf );
        if( *progress > 0 ) { jolt_gui_scr_loadingbar_update( bar, NULL, buf, -1 ); }
    }
}

static void test_autoloading_task( void *param )
{
    jolt_gui_obj_t *scr = (jolt_gui_obj_t *)param;
    int8_t *progress    = jolt_gui_scr_loadingbar_progress_get( scr );
    ESP_LOGI( TAG, "Progress: %d", *progress );
    vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    for( uint8_t i = 0; i <= 100; i += 10 ) {
        *progress = i;
        ESP_LOGI( TAG, "Progress: %d", *progress );
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
    vTaskDelete( NULL );
}

void jolt_gui_test_autoloading_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr = jolt_gui_scr_loadingbar_create( "Autoloading Test" );
        jolt_gui_scr_set_event_cb( scr, test_autoloading_cb );
        jolt_gui_scr_loadingbar_autoupdate( scr );
        jolt_gui_scr_loadingbar_update( scr, NULL, "Initializing", 0 );

        if( NULL == scr ) { ESP_LOGE( TAG, "NULL Loading Screen" ); }
        xTaskCreate( test_autoloading_task, "TestAutoloading", 4096, (void *)scr, 10, NULL );
    }
}

/* Screen that gives info on the battery */
lv_task_t *test_battery_task_h   = NULL;
jolt_gui_obj_t *test_battery_scr = NULL;

void jolt_gui_test_battery_del( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.cancel == event ) {
        lv_task_del( test_battery_task_h );
        if( NULL != test_battery_scr ) {
            lv_obj_del( test_battery_scr );
            test_battery_scr = NULL;
        }
    }
}
void jolt_gui_test_battery_task( lv_task_t *task )
{
    if( NULL != test_battery_scr ) {
        lv_obj_del( test_battery_scr );
        test_battery_scr = NULL;
    }
    int val = adc1_get_raw( JOLT_ADC1_VBATT );
    char buf[40];
    snprintf( buf, sizeof( buf ), "Raw Value: %d\nPercentage: %d", val,
              statusbar_indicators[JOLT_HW_MONITOR_INDEX_BATTERY].val );
    test_battery_scr = jolt_gui_scr_text_create( "Battery", buf );
    jolt_gui_scr_set_event_cb( test_battery_scr, jolt_gui_test_battery_del );
}

void jolt_gui_test_battery_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        test_battery_task_h = lv_task_create( jolt_gui_test_battery_task, 300, LV_TASK_PRIO_LOW, NULL );
    }
}

void jolt_gui_test_alphabet_create( jolt_gui_obj_t *list_btn, jolt_gui_event_t event )
{
    /* Dummy Text Page for Testing */
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr = jolt_gui_scr_text_create(
                "Alphabet",
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ "
                "abcdefghijklmnopqrstuvwxyz "
                "1234567890 "
                "!@#$%^&*()-+_="
                "{}[];':\",.<>?/\\"
                " ¡¢£¤¥¦§¨©ª«¬®¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"
                "The Quick Brown Fox, Jumped Over The Lazy Dog.\n"
                "Testing proper wrap around of very long words and address. "
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ." );
        jolt_gui_scr_scroll_add_monospace_text( scr, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ" );
    }
}

static void https_cb( int16_t status_code, char *post_response, void *params, jolt_gui_obj_t *scr )
{
    if( 200 == status_code ) {
        printf( "Response:\n" );
        printf( post_response );
        printf( "\n" );
    }
    if( NULL != post_response ) { free( post_response ); }
    jolt_gui_obj_del( scr );
}

void jolt_gui_test_https_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr    = jolt_gui_scr_preloading_create( "Test", "HTTPS Test" );
        const char post_data[] = "{ \"action\" : \"block_count\"}";
        jolt_network_post( post_data, https_cb, NULL, scr );
    }
}

void jolt_gui_test_bignum_create( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr = jolt_gui_scr_bignum_create( "BigNum Test", "subtitle", 234567, 6 );
        if( NULL == scr ) ESP_LOGE( TAG, "Failed to create BigNum test screen." );
    }
}
