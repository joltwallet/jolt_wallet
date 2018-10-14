/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"


#include "easy_input.h"

#include "lvgl/lvgl.h"
#include "hal/lv_drivers/display/ssd1306.h"
#include "jolt_gui/jolt_gui.h"

#if 0
#include "gui/gui.h"
#include "gui/first_boot.h"
#include "vault.h"
#include "console.h"
#include "helpers.h"
#include "gui/statusbar.h"
#endif
//#include "radio/wifi.h"
#include "globals.h"
//#include "hal/storage/storage.h"
#include "hal/i2c.h"
#include "syscore/filesystem.h"


static void lv_tick_task(void);

// Definitions for variables in globals.h
volatile QueueHandle_t input_queue;

static const char TAG[] = "main";

static ssd1306_t disp_conf = {
	.protocol = SSD1306_PROTO_I2C,
	//.screen = SH1106_SCREEN,
	.screen = SSD1306_SCREEN,
	.i2c_dev = CONFIG_JOLT_DISPLAY_ADDRESS,
	.rst_pin = CONFIG_JOLT_DISPLAY_PIN_RST,
	.width = 128,
	.height = 64
};

static void display_init() {
    /* Set reset pin as output */
	gpio_config_t io_config;
	io_config.pin_bit_mask = (1 << CONFIG_JOLT_DISPLAY_PIN_RST);
	io_config.mode         = GPIO_MODE_OUTPUT;
	io_config.pull_up_en   = GPIO_PULLUP_DISABLE;
	io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
	io_config.intr_type    = GPIO_INTR_DISABLE;
	ESP_ERROR_CHECK(gpio_config(&io_config));


    ssd1306_init(&disp_conf);

    /*inverse screen (180°) */
#if CONFIG_JOLT_DISPLAY_FLIP
    ssd1306_set_scan_direction_fwd(&disp_conf, true);
    ssd1306_set_segment_remapping_enabled(&disp_conf, false);
#endif

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.disp_flush = ssd1306_flush;
    disp_drv.disp_fill = ssd1306_fill;
    disp_drv.disp_map = ssd1306_map;
    lv_disp_drv_register(&disp_drv);
}

static uint32_t last_key;
static bool easy_input_read(lv_indev_data_t *data) {
    data->state = LV_INDEV_STATE_REL;

    uint64_t input_buf;
    if(xQueueReceive(input_queue, &input_buf, 0)) {
        data->state = LV_INDEV_STATE_PR;
		if(input_buf & (1ULL << EASY_INPUT_BACK)){
            ESP_LOGD(TAG, "back");
            lv_group_send_data(jolt_gui_store.group.back, LV_GROUP_KEY_ENTER);
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
            data->key = LV_GROUP_KEY_ENTER;
		}
        else {
        }
        if( xQueuePeek(input_queue, &input_buf, 0) ) {
            return true;
        }
    }

    return false;
}

static void indev_init() {
    /* Setup Input Button Debouncing Code */
    easy_input_queue_init((QueueHandle_t *)&input_queue);
    easy_input_run( (QueueHandle_t *)&input_queue );

    jolt_gui_group_create();

    lv_indev_t *indev;
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read = easy_input_read;

    indev = lv_indev_drv_register(&indev_drv);
    lv_indev_set_group(indev, jolt_gui_store.group.main);

    return indev;
}

#ifndef UNIT_TESTING
// So our app_main() doesn't override the unit test app_main()
void app_main() {
    /* Setup and Install I2C Driver and supporting objects */
    i2c_driver_setup();

    /* Initialize LVGL graphics system */
    lv_init();
    display_init();
    indev_init();

    /* Run Key/Value Storage Initialization */
    //storage_startup();

    //u8g2_SetContrast( u8g2, get_display_brightness() );

    // Initialize Wireless
    /* todo: this must be before first_boot_setup otherwise attempting
     * to get ap_info before initializing wifi causes a boot loop. investigate
     * more robust solutions */
    esp_log_level_set("wifi", ESP_LOG_NONE);
    //set_jolt_cast();
    //wifi_connect();

    // Allocate space for the vault and see if a copy exists in NVS
#if 0
    if( false == vault_setup()) {
        first_boot_menu();
    }
#endif

    // ==== Initialize the file system ====
    filesystem_init();

    /* Register lv_tick_task after initializing filesystem because initial
     * SPIFFS formatting temporarily disables cache */
    esp_register_freertos_tick_hook(lv_tick_task);

    /* Create GUI */

    //Create main screen obj
    ssd1306_set_whole_display_lighting(&disp_conf, false);
    lv_obj_t * scr = lv_obj_create(NULL, NULL);
    lv_scr_load(scr);


    ESP_LOGI(TAG, "Creating GUI");
    jolt_gui_create();

    // Initiate Console
#if 0
    initialize_console();
    start_console();
#endif

    // LittleVGL Task Handler
    for( ;; vTaskDelay(1) ) {
        lv_task_handler();
        if(ssd1306_need_redraw()) {
            ssd1306_load_frame_buffer(&disp_conf);
        }
    }
}
#endif

static void lv_tick_task(void) {
    lv_tick_inc(portTICK_RATE_MS);
}
