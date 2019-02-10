#include "driver/ledc.h"
#include "led.h"
#include "esp_err.h"
#include "esp_log.h"

/* Setup timers and all that stuff */
void jolt_led_setup() {
#if 1
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = JOLT_TOUCH_LED_RESOLUTION, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = JOLT_TOUCH_LED_MODE,           // timer mode
        .timer_num = JOLT_TOUCH_LED_CHANNEL            // timer index
    };
    ESP_ERROR_CHECK( ledc_timer_config(&ledc_timer) );

    ledc_channel_config_t ledc_channel = {
        .channel    = JOLT_TOUCH_LED_CHANNEL,
        .duty       = 100,
        .gpio_num   = JOLT_HAL_LED_PIN,
        .speed_mode = JOLT_TOUCH_LED_MODE,
        .timer_sel  = JOLT_TOUCH_LED_TIMER,
        .intr_type  = LEDC_INTR_DISABLE,
    };
    ESP_ERROR_CHECK( ledc_channel_config(&ledc_channel) );
#endif
}

/* Set current setting 0~100 % */
void jolt_led_set(uint8_t val) {
    ledc_set_duty(JOLT_TOUCH_LED_MODE, JOLT_TOUCH_LED_CHANNEL, val);
    ledc_update_duty(JOLT_TOUCH_LED_MODE, JOLT_TOUCH_LED_CHANNEL);
}
