#include "led.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/storage/storage.h"

void jolt_led_setup()
{
    {
        ledc_timer_config_t ledc_timer = {
                .duty_resolution = JOLT_TOUCH_LED_RESOLUTION,
                .freq_hz         = JOLT_TOUCH_LED_FREQ_HZ,
                .speed_mode      = JOLT_TOUCH_LED_MODE,
                .timer_num       = JOLT_TOUCH_LED_CHANNEL,
        };
        ESP_ERROR_CHECK( ledc_timer_config( &ledc_timer ) );
    }

    {
        uint8_t val;
        storage_get_u8( &val, "user", "led_val", CONFIG_JOLT_TOUCH_LED_DEFAULT_DUTY );

        ledc_channel_config_t ledc_channel = {
                .channel    = JOLT_TOUCH_LED_CHANNEL,
                .duty       = val,
                .gpio_num   = CONFIG_JOLT_TOUCH_LED_PIN,
                .speed_mode = JOLT_TOUCH_LED_MODE,
                .timer_sel  = JOLT_TOUCH_LED_TIMER,
                .intr_type  = LEDC_INTR_DISABLE,
        };
        ESP_ERROR_CHECK( ledc_channel_config( &ledc_channel ) );
    }
}

/* Set current setting 0~100 % */
void jolt_led_set( uint8_t val )
{
    ledc_set_duty( JOLT_TOUCH_LED_MODE, JOLT_TOUCH_LED_CHANNEL, val );
    ledc_update_duty( JOLT_TOUCH_LED_MODE, JOLT_TOUCH_LED_CHANNEL );
}
