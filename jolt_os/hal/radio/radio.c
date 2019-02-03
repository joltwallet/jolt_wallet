#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_phy_init.h"
#include "stdint.h"
#include "esp_log.h"

#if 0
#include "esp_phy_init.h"
#include "esp32/phy.h"
#include "esp32/phy_init_data.h"
#endif
#include <sys/lock.h>

//MODEM_BLE_MODULE
//MODEM_WIFI_STATION_MODULE

/* todo; prevent other stuff from calling esp_modem_sleep_exit */

static const char TAG[] = "radio.h";
static portMUX_TYPE radio_lock = portMUX_INITIALIZER_UNLOCKED;

static bool reinit_wifi = false;
static bool reinit_bt   = false;

#define JOLT_RADIO_CONTROL_ENABLE 0

/* None of these are well tested.
 *
 * 1 - deinitializes phy rf
 * 2 - puts modem to sleep and holds onto the sleep lock; requires a esp_modem_sleep_get_lock() function in esp_phy_init.h
 * 0 - disabled
 */

void jolt_radio_off() {
    ESP_LOGI(TAG, "Turning off radios");
#if JOLT_RADIO_CONTROL_ENABLE
    esp_err_t err;
    reinit_wifi = false;
    reinit_wifi = true;

    err = esp_phy_rf_deinit(PHY_BT_MODULE);
    if( ESP_OK == err ) {
        reinit_bt = true;
    }
    else {
        ESP_LOGE( TAG, "%s: esp_phy_rf_deinit BT failed with err %s", __func__, esp_err_to_name(err));
    }

    err = esp_phy_rf_deinit(PHY_WIFI_MODULE);
    if( ESP_OK == err ) {
        reinit_wifi = true;
    }
    else {
        ESP_LOGE( TAG, "%s: esp_phy_rf_deinit WIFI failed with err %s", __func__, esp_err_to_name(err));
    }
#elif JOLT_RADIO_CONTROL_ENABLE == 2
    _lock_t s_modem_sleep_lock = esp_modem_sleep_get_lock();
    _lock_acquire( &s_modem_sleep_lock );
    portENTER_CRITICAL( &radio_lock );
    _lock_release( &s_modem_sleep_lock ); /* prevents deadlocks */
    esp_modem_sleep_enter(MODEM_BLE_MODULE); // todo: for loop
    _lock_acquire( &s_modem_sleep_lock );
    portEXIT_CRITICAL( &radio_lock );
#else
    /* Do nothing; currently a stop and not well tested */
#endif
}

void jolt_radio_on() {
    ESP_LOGI(TAG, "Turning on radios");
#if JOLT_RADIO_CONTROL_ENABLE == 1
    esp_err_t err;
    if( reinit_bt ) {
        err = esp_phy_rf_init(NULL,PHY_RF_CAL_NONE,NULL, PHY_BT_MODULE);
        if( ESP_OK != err ) {
            ESP_LOGE( TAG, "%s: esp_phy_rf_init BT failed with err %s", __func__, esp_err_to_name(err));
        }
    }

    if( reinit_wifi ) {
        err = esp_phy_rf_init(NULL,PHY_RF_CAL_NONE,NULL, PHY_WIFI_MODULE);
        if( ESP_OK != err ) {
            ESP_LOGE( TAG, "%s: esp_phy_rf_init WIFI failed with err %s", __func__, esp_err_to_name(err));
        }
    }

    err = esp_phy_rf_init(NULL,PHY_RF_CAL_NONE,NULL, PHY_MODEM_MODULE);
    if( ESP_OK != err ) {
        ESP_LOGE( TAG, "%s: esp_phy_rf_init MODEM failed with err %s", __func__, esp_err_to_name(err));
    }

    reinit_wifi = false;
    reinit_bt = false;
#elif JOLT_RADIO_CONTROL_ENABLE == 2
    _lock_t s_modem_sleep_lock = esp_modem_sleep_get_lock();
    _lock_release( &s_modem_sleep_lock );
#else
    /* Do nothing; currently a stop and not well tested */

#endif

}

