/**
 * @file ota.h
 * @brief High level JoltOS update functions
 * @author Brian Pugh
 */

#ifndef JOLT_SYSCORE_OTA_H__
#define JOLT_SYSCORE_OTA_H__

#include "esp_ota_ops.h"


/**
 * @brief Initialize the OTA moduel
 * @return ESP_OK on success
 */
esp_err_t jolt_ota_init_handle( );

/**
 * @brief Perform a streaming JoltOS update via YMODEM
 * @param[out] progress YMODEM transfer progress [0~100]
 * @return ESP_OK on success
 */
esp_err_t jolt_ota_ymodem(int8_t *progress);

#endif
