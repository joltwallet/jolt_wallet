#ifndef JOLT_SYSCORE_HTTPS_H__
#define JOLT_SYSCORE_HTTPS_H__

#include "esp_err.h"
#include "esp_http_client.h" 

#define JOLT_NETWORK_TIMEOUT_MS 10000

/* status_code: negative if a local error (e.g. OOM) occurs. Otherwise will be the
 * http response code.
 *
 * response: null-terminated, allocated string of the server response. 
 * It is the user's responsibility to free this memory.
 */
typedef void (*jolt_network_client_cb_t)(int16_t status_code, char *response);

/* initializes http client. Must be called before jolt_network_post. 
 * Can be called multiple times to change URI.*/
esp_err_t jolt_network_client_init( char *uri );

/* Calls post command.
 *
 * Returns ESP_OK on successful job creation.
 * Returns ESP_FAIL on job failure. This could be due to OOM or a full job queue.*/
esp_err_t jolt_network_post( const char *post_data, jolt_network_client_cb_t cb );

#endif
