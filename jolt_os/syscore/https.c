#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "string.h"
#include "esp_log.h"

#include "bg.h"
#include "hal/storage/storage.h"
#include "https.h"


static const char TAG[] = "https";

static esp_http_client_handle_t client = NULL;
static SemaphoreHandle_t client_mutex = NULL;

/**
 * @brief variables for the http_event_handler to ineract with
 */
typedef struct {
    uint32_t jid;           /** monotonically increasing job_id */
    uint32_t jid_cancelled; /** JID to cancel */
    uint32_t len;           /**< length of allocated response buffer */
    uint32_t written;       /**< how many bytes have been written to response */
    char **response;        /**< Pointer of where to store allocated data pointer */
} cb_data_t;

static cb_data_t cb_data = {
    .jid_cancelled = UINT32_MAX,
};

typedef struct {
    jolt_network_client_cb_t cb;
    char *post_data;
    esp_http_client_method_t method;
    void *param;                     // Additional parameters to be fed to the user cb 
    bool running;                    /**< True if job is currently running */
} https_job_t;

static esp_err_t _http_event_handle(esp_http_client_event_t *evt) {
    char *response = *(cb_data.response);
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER");
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            if (esp_http_client_is_chunked_response(evt->client)) {
                // todo: handle chunked response
            }
            if(cb_data.jid == cb_data.jid_cancelled) {
                /* job has been cancelled */
                ESP_LOGI(TAG, "Not copying data; job was cancelled");
                free(response);
                break;
            }
            if(NULL == response) {
                cb_data.len = evt->data_len + 1; // for null terminator
                response = malloc(cb_data.len);
                if( NULL == response ) {
                    ESP_LOGE(TAG, "Unable to alloc response data");
                    // todo error handling
                }
                cb_data.written = 0;
            }
            else{
                cb_data.len += evt->data_len;
                response = realloc(response, cb_data.len);
                if( NULL == response ) {
                    ESP_LOGE(TAG, "Unable to realloc response data");
                    // todo error handling
                }
            }
            memcpy(&response[cb_data.written], evt->data, evt->data_len);
            cb_data.written += evt->data_len;
            response[cb_data.written] = '\0';
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            cb_data.jid++;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    *cb_data.response = response;
    return ESP_OK;
}

static int https_func( jolt_bg_job_t *bg_job ) {
    https_job_t *job = jolt_bg_get_param( bg_job );
    lv_obj_t *scr = jolt_bg_get_scr( bg_job );
    char *response = NULL;
    cb_data.response = &response;

    int16_t status_code = JOLT_NETWORK_ERROR;
    esp_err_t err;

    /* First call */
    if( !job->running  ) {
        /* Try and grab https semaphore */
        if( pdFALSE == xSemaphoreTake( client_mutex, 0) ) {
            return 100; /* Try again in 100mS */
        }
        job->running = true;
        /* Set HTTP Method */
        esp_http_client_set_method(client, job->method);

        /* If it's a post method, set the post field */
        if(HTTP_METHOD_POST == job->method) {
            err = esp_http_client_set_post_field(client, job->post_data, strlen(job->post_data));
            if( ESP_OK != err ) {
                ESP_LOGE(TAG, "Failed to set http post field");
                goto exit;
            }
            else {
                ESP_LOGI(TAG, "Set POST:\n%s\n", job->post_data);
            }
        }
    }

    /* Poll the http client until complete or user abort */
    err = esp_http_client_perform(client);

    if( ESP_OK == err ) {
        ESP_LOGD(TAG, "https client perform success.\n%s\n", response);
    }
    else if( ESP_FAIL == err ) {
        ESP_LOGE(TAG, "https client failed.");
        goto exit;
    }
    else if( JOLT_BG_ABORT == jolt_bg_get_signal( bg_job ) ) {
        ESP_LOGI(TAG, "user aborted https client.");
        status_code = JOLT_NETWORK_CANCEL;
        cb_data.jid_cancelled = cb_data.jid;
        goto exit;
    }
    else {
        return 100; /* Try again in 100mS */
    }

    status_code = esp_http_client_get_status_code(client);
    if( 200 != status_code ) {
        ESP_LOGW(TAG, "Https client returned an unsuccessful status_code %d", status_code);
        goto exit;
    }

    if(esp_http_client_is_chunked_response(client)) {
        ESP_LOGW(TAG, "Chunked response logic not yet implemented.");
    }

    /* Call user callback and free job resources */
exit:
    free(job->post_data);
    xSemaphoreGive( client_mutex );
    ESP_LOGI(TAG, "Calling user callback");
    if( NULL != job->cb ) {
        job->cb(status_code, response, job->param, scr);
    }
    free( job );
    return 0;
}

esp_err_t jolt_network_client_init_from_nvs() {
    size_t required_size;
    char *uri = NULL;
    esp_err_t err;

    /* Sets Jolt Cast Server Params from NVS*/
    storage_get_str(NULL, &required_size, "user", "jc_uri", CONFIG_JOLT_CAST_URI);
    if( NULL == (uri = malloc(required_size)) ) {
        err = jolt_network_client_init( CONFIG_JOLT_CAST_URI );
        goto exit;
    }
    storage_get_str(uri, &required_size, "user", "jc_uri", CONFIG_JOLT_CAST_URI);
    err = jolt_network_client_init( uri );
    if( ESP_OK != err ){
        ESP_LOGE(TAG, "Failed using JoltCast URI from NVS:\n%s\nUsing default URI.", uri);
        err = jolt_network_client_init( CONFIG_JOLT_CAST_URI );
        goto exit;
    }

exit:
    SAFE_FREE(uri);
    return err;
}

esp_err_t jolt_network_client_init( char *uri ) {
    static char *local_uri = NULL;

    if( NULL == uri ){
        ESP_LOGE(TAG, "Must specify URI!");
        return ESP_FAIL;
    }

    /* Create client-use mutex */
    if( NULL == client_mutex ) {
        client_mutex = xSemaphoreCreateMutex();
        if( NULL == client_mutex ) {
            ESP_LOGE(TAG, "Failed to create client_mutex.");
            return ESP_FAIL;
        }
    }

    /* Allocate a static copy of the uri */
    {
        char *tmp = NULL;
        size_t uri_buf_len = strlen(uri) + 1; 
        tmp = malloc( uri_buf_len );
        if( NULL == tmp ){
            ESP_LOGE(TAG, "Failed to allocate space for URI");
            return ESP_FAIL;
        }
        strlcpy(tmp, uri, uri_buf_len);
        if( NULL != local_uri ) {
            free(local_uri);
        }
        local_uri = tmp;
    }

    if( NULL == client ) { /* Configure http client */
        esp_http_client_config_t config = {
            /* todo: provide certificate in PEM format */ 
            //.cert_pem = howsmyssl_com_root_cert_pem_start,
            .timeout_ms = JOLT_NETWORK_TIMEOUT_MS,
            //.buffer_size = 2048,
            .is_async   = true,
            .event_handler = _http_event_handle,
        };
        config.url = local_uri;
        client = esp_http_client_init( &config );
        if( NULL == client ) {
            ESP_LOGE(TAG, "Failed to create https client");
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

/* post_data must be a json_formatted, null-terminated ascii string.
 *
 * Allocates memory and copies post_data so that post_data doesn't need to exist
 * after calling. 
 *
 * The user is responsible for freeing the response buffer.*/
esp_err_t jolt_network_post( const char *post_data, jolt_network_client_cb_t cb, void *param, lv_obj_t *scr ) {
    char *cmd = NULL; // for allocated copy of post_data
    https_job_t *job = NULL;

    if( NULL == cb ) {
        ESP_LOGE(TAG, "A callback must be specified");
        goto exit;
    }

    /* Check to make sure https network objects are initialized.
     *     We can't just initialize it here because a URI must be provided. 
     */
    if( NULL == client ) {
        ESP_LOGE(TAG, "You must first call jolt_network_client_init");
        goto exit;
    }

    /* Allocate memory for the POST command */
    cmd = strdup(post_data);
    if( NULL == cmd ) {
        ESP_LOGE(TAG, "Failed to allocate memory for post_data cmd.");
        goto exit;
    }

    /* Allocate memory for the https job */
    job = malloc( sizeof(https_job_t) );
    if( NULL == job ) {
        ESP_LOGE(TAG, "Failed to allocate memory for https job");
        goto exit;
    }

    /* Set all job parameters */
    job->cb = cb;
    job->post_data = cmd;
    job->method = HTTP_METHOD_POST;
    job->param = param;
    job->running = false;

    /* Send the job to the job_queue. Do NOT block to put it on the queue */
    if(ESP_OK != jolt_bg_create( https_func, job, scr) ) {
        ESP_LOGE(TAG, "Failed to add https job to the queue");
        goto exit;
    }

    return ESP_OK;

exit:
    if( NULL != cmd ){
        free(cmd);
    }
    if( NULL != job) {
        free( job );
    }
    return ESP_FAIL;
}
