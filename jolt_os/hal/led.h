#ifndef JOLT_HAL_LED_H__
#define JOLT_HAL_LED_H__

#define JOLT_TOUCH_LED_MODE LEDC_LOW_SPEED_MODE
#define JOLT_TOUCH_LED_TIMER LEDC_TIMER_0
#define JOLT_TOUCH_LED_CHANNEL LEDC_CHANNEL_0
#define JOLT_TOUCH_LED_RESOLUTION LEDC_TIMER_8_BIT
#define JOLT_TOUCH_LED_FREQ_HZ 1000

void jolt_led_setup();

/* Set current setting 0~100 % */
void jolt_led_set(uint8_t val);

#endif
