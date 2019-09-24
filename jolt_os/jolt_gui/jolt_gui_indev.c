//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "jolt_gui_indev.h"
#include "easy_input.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "jolt_gui/jolt_gui.h"
#include "stdbool.h"

static const char TAG[] = "jolt_gui_indev";

volatile uint64_t *jolt_btn_state = &easy_input_state;

#if UNIT_TESTING
static volatile uint64_t jolt_btn_sim_state = 0;
#endif

/**
 * @brief Function thats periodically read to get the state of user input
 *
 * Must populate the following
 * data->uint32_t key;              // For LV_INDEV_TYPE_KEYPAD the currently pressed key
 * data->lv_indev_state_t state;    // LV_INDEV_STATE_REL or LV_INDEV_STATE_PR
 *
 * @param[in,out] indev_drv
 * @param[out] data Populated with the current keypad state
 */
static bool easy_input_read( lv_indev_drv_t *indev_drv, lv_indev_data_t *data )
{
    uint64_t state;
    state = *jolt_btn_state;

#if UNIT_TESTING
    state |= jolt_btn_sim_state;
#endif

    if( state ) {
        data->state = LV_INDEV_STATE_PR;

        if( state & ( 1ULL << JOLT_BTN_BACK ) ) {
            ESP_LOGD( TAG, "back" );
            data->key = LV_KEY_ESC;
        }
        else if( state & ( 1ULL << JOLT_BTN_UP ) ) {
            ESP_LOGD( TAG, "up" );
            data->key = LV_KEY_UP;
        }
        else if( state & ( 1ULL << JOLT_BTN_DOWN ) ) {
            ESP_LOGD( TAG, "down" );
            data->key = LV_KEY_DOWN;
        }
        else if( state & ( 1ULL << JOLT_BTN_ENTER ) ) {
            ESP_LOGD( TAG, "enter" );
            data->key = LV_KEY_ENTER;
        }
        else {
        }
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }

    return false; /* No buffering */
}

void jolt_gui_indev_init()
{
    /* Setup Input Button Debouncing Code */
    lv_group_t *group;

    easy_input_run( NULL );

    jolt_gui_group_create();

    lv_indev_t *indev;
    lv_indev_drv_t indev_drv;

    lv_indev_drv_init( &indev_drv );
    indev_drv.type    = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = easy_input_read;

    group = jolt_gui_group_get();

    indev = lv_indev_drv_register( &indev_drv );
    lv_indev_set_group( indev, group );
}

#if UNIT_TESTING

static void btn_timer_cb( void *arg )
{
    jolt_btn_t btn = ( jolt_btn_t )(uint32_t)arg;
    jolt_btn_sim_state &= ~( 1ULL << btn );
}

void jolt_btn_press( jolt_btn_t btn, uint32_t duration, bool blocking )
{
    esp_timer_handle_t handle    = NULL;
    esp_timer_create_args_t args = {0};
    esp_err_t err;

    if( 0 == duration ) duration = JOLT_BTN_DEFAULT_DURATION_MS;

    if( blocking ) {
        jolt_btn_sim_state |= ( 1ULL << btn );
        vTaskDelay( pdMS_TO_TICKS( duration ) );
        jolt_btn_sim_state &= ~( 1ULL << btn );
    }
    else {
        args.callback = btn_timer_cb;
        args.arg      = (void *)(uint32_t)btn;

        err = esp_timer_create( &args, &handle );
        if( ESP_OK != err ) {
            ESP_LOGE( TAG, "Failed to create btn timer" );
            goto exit;
        }

        jolt_btn_sim_state |= ( 1 << btn );

        err = esp_timer_start_once( handle, duration * 1000 );
        if( ESP_OK != err ) {
            ESP_LOGE( TAG, "Failed to start timer" );
            goto exit;
        }
    }

    return;

exit:
    jolt_btn_sim_state &= ~( 1 << btn );
    if( handle ) esp_timer_delete( handle );
}

#endif
