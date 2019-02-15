#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "string.h"
#include "esp_log.h"
#include "https.h"

#include "bg.h"


static const char TAG[] = "jolt_https";

static esp_http_client_handle_t client = NULL;

/* todo: job queue */
typedef struct {
    jolt_network_client_cb_t cb;
    char *post_data;
    esp_http_client_method_t method;
    void *param;                     // Additional parameters to be fed to the user cb 
} https_job_t;

static void https_func( jolt_bg_job_t *bg_job ) {
    https_job_t *job = jolt_bg_get_param( bg_job );
    lv_obj_t *scr = jolt_bg_get_scr( bg_job );

    int16_t status_code = -1;
    char *response = NULL;
    esp_err_t err;
    /* Set HTTP Method */
    esp_http_client_set_method(client, job->method);

    /* If it's a post method, set the post field */
    if(HTTP_METHOD_POST == job->method) {
        err = esp_http_client_set_post_field(client, job->post_data, strlen(job->post_data));
        if( ESP_OK != err ) {
            ESP_LOGE(TAG, "Failed to set http post field");
            goto exit;
        }
    }

    /* Poll the http client until complete or user abort */
    for(;; vTaskDelay(pdMS_TO_TICKS(77))) {
        err = esp_http_client_perform(client);

        if( ESP_OK == err ) {
            ESP_LOGD(TAG, "https client perform success.");
            break;
        }
        else if( ESP_FAIL == err ) {
            ESP_LOGE(TAG, "https client failed.");
            goto exit;
        }
        else if( JOLT_BG_ABORT == jolt_bg_get_signal( bg_job ) ) {
            ESP_LOGI(TAG, "user aborted https client.");
            goto exit;
        }
    }

    status_code = esp_http_client_get_status_code(client);
    if( 200 != status_code ) {
        ESP_LOGI(TAG, "Https client returned an unsuccessfully status_code");
        goto exit;
    }

    { /* Copy response */
        int content_length = esp_http_client_get_content_length( client );
        response = malloc(content_length + 1);
        if( NULL == response ) {
            status_code = -2;
            goto exit;
        }
        int read_len = esp_http_client_read(client, response, content_length);
        response[read_len] = '\0';
    }

    /* Call user callback and free job resources */
exit:
    ESP_LOGI(TAG, "Calling user callback");
    job->cb(status_code, response, job->param, scr);
    free( job );
}

/* Initializes all networking objects (if necessary). Will copy uri to a local
 * buffer.
 * Returns ESP_OK on success. */
esp_err_t jolt_network_client_init( char *uri ) {
    static char *local_uri = NULL;

    if( NULL == uri ){
        ESP_LOGE(TAG, "Must specify URI!");
        return ESP_FAIL;
    }

    /* Allocate a static copy of the uri */
    {
        char *tmp = NULL;
        tmp = malloc( strlen(uri) + 1 );
        if( NULL == tmp ){
            ESP_LOGE(TAG, "Failed to allocate space for URI");
            return ESP_FAIL;
        }
        strcpy(tmp, uri);
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
            .is_async   = true,
        };
        /* todo: use NVS params and update current references */
        config.url = "https://yapraiwallet.space/quake/api";
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
    cmd = malloc(strlen(post_data) + 1);
    if( NULL == cmd ) {
        ESP_LOGE(TAG, "Failed to allocate memory for post_data cmd.");
        goto exit;
    }
    strcpy(cmd, post_data);

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
