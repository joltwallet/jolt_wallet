#include "driver/ledc.h"
#include "led.h"
#include "esp_err.h"
#include "esp_log.h"
#define JOLT_HAL_LED_PIN 34
#define JOLT_TOUCH_LED_MODE LEDC_LOW_SPEED_MODE
#define JOLT_TOUCH_LED_TIMER LEDC_TIMER_0
#define JOLT_TOUCH_LED_CHANNEL LEDC_CHANNEL_0

/* Setup timers and all that stuff */
void jolt_led_setup() {
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = JOLT_TOUCH_LED_RESOLUTION, // resolution of PWM duty
        .freq_hz = 1000,                      // frequency of PWM signal
        .speed_mode = JOLT_TOUCH_LED_MODE,           // timer mode
        .timer_num = JOLT_TOUCH_LED_CHANNEL            // timer index
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .channel    = JOLT_TOUCH_LED_CHANNEL,
        .duty       = 0,
        .gpio_num   = JOLT_HAL_LED_PIN,
        .speed_mode = JOLT_TOUCH_LED_MODE,
        .hpoint     = 0,
        .timer_sel  = JOLT_TOUCH_LED_TIMER
    };
    ledc_channel_config(&ledc_channel);
}

/* Set current setting 0~100 % */
void jolt_led_set(uint8_t val) {
    ledc_set_duty_and_update(JOLT_TOUCH_LED_MODE, JOLT_TOUCH_LED_CHANNEL, val, 0);
}
