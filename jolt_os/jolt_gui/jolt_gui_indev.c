//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG
//
#include "esp_log.h"
#include "stdbool.h"
#include "jolt_gui/jolt_gui.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "easy_input.h"

static const char TAG[] = __FILE__;

static QueueHandle_t input_queue;

static bool easy_input_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    data->state = LV_INDEV_STATE_REL;

    uint64_t input_buf;
    if(xQueueReceive(input_queue, &input_buf, 0)) {
        data->state = LV_INDEV_STATE_PR;
        if(input_buf & (1ULL << EASY_INPUT_BACK)){
            ESP_LOGD(TAG, "back");
            jolt_gui_send_enter_back( NULL );
        }
        else if(input_buf & (1ULL << EASY_INPUT_UP)){
            ESP_LOGD(TAG, "up");
            data->key = LV_GROUP_KEY_UP;
        }
        else if(input_buf & (1ULL << EASY_INPUT_DOWN)){
            ESP_LOGD(TAG, "down");
            data->key = LV_GROUP_KEY_DOWN;
        }
        else if(input_buf & (1ULL << EASY_INPUT_ENTER)){
            ESP_LOGD(TAG, "enter");
            jolt_gui_send_enter_enter( NULL );
        }
        else {
        }
        if( xQueuePeek(input_queue, &input_buf, 0) ) {
            return true;
        }
    }

    return false;
}

void jolt_gui_indev_init() {
    /* Setup Input Button Debouncing Code */
    lv_group_t *group;

    easy_input_queue_init( &input_queue );
    easy_input_run( &input_queue );

    jolt_gui_group_create();

    lv_indev_t *indev;
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = easy_input_read;

    group = jolt_gui_group_main_get();

    indev = lv_indev_drv_register( &indev_drv );
    lv_indev_set_group(indev, group);
}

