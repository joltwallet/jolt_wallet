#include "jolt_gui/jolt_gui.h"
#include "jolt_helpers.h"
#include "hal/display.h"
#include "hal/storage/storage.h"

#define BUF_LINES 8
static const char TAG[] = "display.c";
static ssd1306_t disp_hal = { 0 };
static DRAM_ATTR lv_color_t buf[LV_HOR_RES_MAX*BUF_LINES];

void display_init() {
    static lv_disp_drv_t lv_disp_drv;
    static lv_disp_buf_t disp_buf;

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
    disp_hal.width     = LV_HOR_RES_MAX;
    disp_hal.height    = LV_VER_RES_MAX;
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

    lv_disp_drv_init(&lv_disp_drv);
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * BUF_LINES * 8);    /*Initialize the display buffer*/
    lv_disp_drv.buffer = &disp_buf;
    lv_disp_drv.flush_cb = ssd1306_flush;
    lv_disp_drv.set_px_cb = ssd1306_vdb_wr;
    lv_disp_drv.rounder_cb = ssd1306_rounder;
    lv_disp_drv_register(&lv_disp_drv);

    ssd1306_set_whole_display_lighting(&disp_hal, false);
    ssd1306_set_inversion(&disp_hal, true);
    set_display_brightness(get_display_brightness());
}

uint8_t *get_display_buf() {
    assert( BUF_LINES == 8 );
    return (uint8_t*) buf;
}

void print_display_buf() {
    assert( LV_HOR_RES_MAX == 128 );
    assert( BUF_LINES == 8 );
    printf("\n");
    char print_buf[LV_HOR_RES_MAX*4 + 2] = { 0 };  // newline + null-terminator 

    JOLT_GUI_CTX{
        jolt_suspend_logging();
        lv_obj_invalidate(lv_scr_act());
        lv_refr_now(NULL);
        for(uint8_t y1=0; y1 < LV_VER_RES_MAX-1; y1+=2){
            memzero(print_buf, sizeof(print_buf));
            uint8_t y2 = y1 + 1;
            for(uint8_t x=0; x < LV_HOR_RES_MAX; x++){
                bool val1, val2;
                uint8_t *disp_buf_ptr;
                disp_buf_ptr = (uint8_t*)buf + LV_HOR_RES_MAX * (y1 >> 3) + x;
                val1 = (bool)(*disp_buf_ptr & (1<<(y1 % 8)));
                disp_buf_ptr = (uint8_t*)buf + LV_HOR_RES_MAX * (y2 >> 3) + x;
                val2 = (bool)(*disp_buf_ptr & (1<<(y2 % 8)));
                if(val1 && val2) strlcat(print_buf, " ", sizeof(print_buf));
                else if(val1)    strlcat(print_buf, "▄", sizeof(print_buf));
                else if(val2)    strlcat(print_buf, "▀", sizeof(print_buf));
                else             strlcat(print_buf, "█", sizeof(print_buf));
            }
            strlcat(print_buf, "\n", sizeof(print_buf));
            printf(print_buf);
        }
        jolt_resume_logging();
    }
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
    ssd1306_set_precharge_period(&disp_hal, precharge_levels[level]);
}
