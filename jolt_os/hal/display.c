#include "lvgl/lvgl.h"
#include "jolt_helpers.h"
#include "hal/display.h"
#include "hal/storage/storage.h"

static const char TAG[] = "display.c";
static ssd1306_t disp_hal = { 0 };

void display_init() {
    /* Set reset pin as output */
    gpio_config_t io_config;
    io_config.pin_bit_mask = (1 << CONFIG_JOLT_DISPLAY_PIN_RST);
    io_config.mode         = GPIO_MODE_OUTPUT;
    io_config.pull_up_en   = GPIO_PULLUP_DISABLE;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_config.intr_type    = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_config));

    disp_hal.protocol  = SSD1306_PROTO_I2C;
    disp_hal.screen    = SSD1306_SCREEN;
    disp_hal.i2c_dev   = CONFIG_JOLT_DISPLAY_ADDRESS;
    disp_hal.rst_pin   = CONFIG_JOLT_DISPLAY_PIN_RST;
    disp_hal.width     = LV_HOR_RES;
    disp_hal.height    = LV_VER_RES;
    ESP_ERROR_CHECK(ssd1306_init(&disp_hal));

    /*inverse screen (180°) */
#if CONFIG_JOLT_DISPLAY_FLIP
    ESP_LOGI(TAG, "Flipping Display");
    ssd1306_set_scan_direction_fwd(&disp_hal, true);
    ssd1306_set_segment_remapping_enabled(&disp_hal, false);
#else
    ssd1306_set_scan_direction_fwd(&disp_hal, false);
    ssd1306_set_segment_remapping_enabled(&disp_hal, true);
#endif

    static lv_disp_drv_t lv_disp_drv;
    lv_disp_drv_init(&lv_disp_drv);
    lv_disp_drv.flush_cb = ssd1306_flush;
    lv_disp_drv.set_px_cb = ssd1306_vdb_wr;
    lv_disp_drv.rounder_cb = ssd1306_rounder;
    lv_disp_drv_register(&lv_disp_drv);

    ssd1306_set_whole_display_lighting(&disp_hal, false);
    ssd1306_set_inversion(&disp_hal, true);
    set_display_brightness(get_display_brightness());
}

static const uint8_t brightness_levels[] = { 0, 1, 2, 50, 120, 255};
static const uint8_t precharge_levels[]  = {10, 10, 10, 90, 130, 255};

uint8_t get_display_brightness() {
    /* Returns saved brightness level or default */
    uint8_t brightness;
    storage_get_u8(&brightness, "user", "disp_bright", CONFIG_JOLT_DISPLAY_BRIGHTNESS);
    ESP_LOGI(TAG,"brightness %d", brightness);
    return brightness;
}

void save_display_brightness(uint8_t level) {
    storage_set_u8(level, "user", "disp_bright");
}

void set_display_brightness(uint8_t level) {
    ssd1306_set_contrast(&disp_hal, brightness_levels[level]);
    ssd1306_set_precharge_period(&disp_hal, brightness_levels[level]);
}
