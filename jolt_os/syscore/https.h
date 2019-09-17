/**
 * @file https.h
 * @brief Handles all JOLT wifi network communications
 * @author Brian Pugh
 */

#ifndef JOLT_SYSCORE_HTTPS_H__
#define JOLT_SYSCORE_HTTPS_H__

#include "esp_err.h"
#include "esp_http_client.h"
#include "jolt_gui/jolt_gui.h"

#define JOLT_NETWORK_TIMEOUT_MS 10000

/**
 * @brief Internal/local network error codes
 */
enum {
    JOLT_NETWORK_ERROR    = -1, /**< Unspecified Error */
    JOLT_NETWORK_TIMEDOUT = -2, /**< Network Timed Out*/
    JOLT_NETWORK_OOM      = -3, /**< Unable to allocate memory */
    JOLT_NETWORK_CANCEL   = -4, /**< User cancelled request */
};
typedef uint8_t jolt_network_status_t;

/**
 * @brief Network Callback
 *
 * @param[in] status_code negative if a local error (e.g. OOM) occurs. Otherwise will be the
 * http response code.
 *
 * @param[in] response null-terminated, allocated string of the server response. It is the user's responsibility to
 * free this memory.
 *
 * @param[in] free param
 *
 * @param[in] scr @see jolt_bg_task
 */
typedef void ( *jolt_network_client_cb_t )( int16_t status_code, char *response, void *param, lv_obj_t *scr );

/**
 * @brief Initializes the HTTPS client module.
 *
 * Initializes the https module and sets the URI for the remote server.
 * Jolt applications are ONLY allowed to communicate with this remote server.
 * Must be called before jolt_network_post.
 * Can be called multiple times to change URI.
 *
 * @param[in] uri Remote server URI; e.g. "https://joltwallet.com/api"
 * @return ESP_OK on success; ESP_FAIL on failure.
 */
esp_err_t jolt_network_client_init( char *uri );

/**
 * @brief Convenience jolt_network_client using URI from NVS
 * @return ESP_OK on success.
 */
esp_err_t jolt_network_client_init_from_nvs();

/**
 * @brief Calls post command; utilizing the background task engine.
 *
 * post_data must be a json_formatted, null-terminated ascii string.
 *
 * jolt_network_most will allocate memory and copy post_data so that the
 * provided post_data string doesn't need to persist after calling.
 *
 * The user is responsible for freeing the response buffer in the provided cb.
 *
 * @param[in] post_data null-terminated, json-formatted POST string.
 * @param[in] cb Callback
 * @param[in] param @see jolt_bg_task
 * @param[in] scr @see jolt_bg_task
 *
 * @return ESP_OK on successful job creation; ESP_FAIL otherwise.
 */
esp_err_t jolt_network_post( const char *post_data, jolt_network_client_cb_t cb, void *param, lv_obj_t *scr );

#endif
