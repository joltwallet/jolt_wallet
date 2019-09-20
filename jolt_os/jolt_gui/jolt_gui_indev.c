//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "easy_input.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "jolt_gui/jolt_gui.h"
#include "stdbool.h"

static const char TAG[] = "jolt_gui_indev";

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
    if( easy_input_state ) {
        data->state = LV_INDEV_STATE_PR;

        if( easy_input_state & ( 1ULL << EASY_INPUT_BACK ) ) {
            ESP_LOGD( TAG, "back" );
            data->key = LV_KEY_ESC;
        }
        else if( easy_input_state & ( 1ULL << EASY_INPUT_UP ) ) {
            ESP_LOGD( TAG, "up" );
            data->key = LV_KEY_UP;
        }
        else if( easy_input_state & ( 1ULL << EASY_INPUT_DOWN ) ) {
            ESP_LOGD( TAG, "down" );
            data->key = LV_KEY_DOWN;
        }
        else if( easy_input_state & ( 1ULL << EASY_INPUT_ENTER ) ) {
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
