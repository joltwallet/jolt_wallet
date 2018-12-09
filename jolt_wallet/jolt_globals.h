/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef JOLT_GLOBALS_H__
#define JOLT_GLOBALS_H__

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "vault.h"
#include "hal/lv_drivers/display/SSD1306.h"

/* Macros so that the battery adc pin can be specified in Kconfig */
#define XJOLT_ADC1(x) ADC1_GPIO ## x ## _CHANNEL
#define JOLT_ADC1(x) XJOLT_ADC1(x)
#define JOLT_ADC1_VBATT JOLT_ADC1(CONFIG_JOLT_VBATT_SENSE_PIN)

vault_t *vault;

extern ssd1306_t disp_hal;
extern TaskHandle_t console_h;

typedef enum release_type_t {
    JOLT_VERSION_RELEASE = 0,
    JOLT_VERSION_DEV = 1,
} release_type_t;

typedef struct jolt_version_t {
    union {
        struct {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
        };
        uint32_t version; // For easy comparison
    };
    release_type_t release;
} jolt_version_t;
extern const jolt_version_t JOLT_VERSION;

#endif
