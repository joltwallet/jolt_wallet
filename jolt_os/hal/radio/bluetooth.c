/* What works:
 *    *streams: ble_stdin, ble_stdout, ble_stderr
 * What doesn't work:
 *    *select
*/
#include "sdkconfig.h"
#include "esp_spiffs.h"

#if CONFIG_BT_ENABLED

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/portmacro.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "string.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include <stdarg.h>
#include <sys/errno.h>
#include <sys/lock.h>
#include <sys/fcntl.h>
#include <sys/param.h>

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"

#include "esp_console.h"
#include "console.h"

#include "hal/radio/spp_recv_buf.h"
#include "linenoise/linenoise.h"
#include "bluetooth.h"
#include "bluetooth_state.h"


#define GATTS_SEND_REQUIRE_CONFIRM false

FILE *ble_stdin;
FILE *ble_stdout;
FILE *ble_stderr;

static void gap_event_handler(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *);
static void gatts_event_handler(esp_gatts_cb_event_t , 
        esp_gatt_if_t, esp_ble_gatts_cb_param_t *);

#define GATTS_TABLE_TAG  "GATTS_SPP_DEMO"
#define TAG  "GATTS_SPP_DEMO"

/***************************
 *  SPP PROFILE ATTRIBUTES *
 ***************************/

/* DRIVER STUFF */
static int     ble_open(const char * path, int flags, int mode );
static int     ble_fstat( int fd, struct stat * st );
static int     ble_close( int fd );
static ssize_t ble_write( int fd, const void * data, size_t size );
static ssize_t ble_read(  int fd, void* data, size_t size );
// Token signifying that no character is available
#define NONE -1

#if 0
static esp_line_endings_t s_tx_mode =
#if CONFIG_NEWLIB_STDOUT_LINE_ENDING_CRLF
        ESP_LINE_ENDINGS_CRLF;
#elif CONFIG_NEWLIB_STDOUT_LINE_ENDING_CR
        ESP_LINE_ENDINGS_CR;
#else
        ESP_LINE_ENDINGS_LF;
#endif
#endif

// Newline conversion mode when receiving
static esp_line_endings_t s_rx_mode =
#if CONFIG_NEWLIB_STDIN_LINE_ENDING_CRLF
        ESP_LINE_ENDINGS_CRLF;
#elif CONFIG_NEWLIB_STDIN_LINE_ENDING_CR
        ESP_LINE_ENDINGS_CR;
#else
        ESP_LINE_ENDINGS_LF;
#endif

// write bytes function type
typedef void (*tx_func_t)(int, int);
// read bytes function type
typedef int (*rx_func_t)(int);

static _lock_t s_ble_read_lock;
static _lock_t s_ble_write_lock;

/* Lock ensuring that uart_select is used from only one task at the time */

static int ble_open(const char * path, int flags, int mode) {
    int fd = -1;
    if (strcmp(path, "/0") == 0) {
        fd = 0;
    }
    else {
        errno = ENOENT;
        return fd;
    }
    return fd;
}

static ssize_t ble_write(int fd, const void *data, size_t size) {
	const char *data_c = (const char *)data;
    _lock_acquire_recursive(&s_ble_write_lock);

    int idx = 0;
    do{
        uint16_t print_len = size;
        if( print_len > 512 ) {
            print_len = 512;
        }
        esp_err_t res;
        res = esp_ble_gatts_send_indicate(
                spp_profile_tab[SPP_PROFILE_A_APP_ID].gatts_if,
                spp_profile_tab[SPP_PROFILE_A_APP_ID].conn_id,
                spp_handle_table[SPP_IDX_SPP_DATA_NOTIFY_VAL],
                print_len, (uint8_t*) &data_c[idx], GATTS_SEND_REQUIRE_CONFIRM);
        if( ESP_OK != res ){
            // todo: res error handling
            esp_restart();
        }
        idx += print_len;
        size -= print_len;
    } while(size>0);

    _lock_release_recursive(&s_ble_write_lock);
    return size;
}

static int peek_c = NONE;
static int ble_read_char(int fd) {
    static uint32_t line_off = 0; // offset into most recent queue item
    if ( peek_c != NONE ){
        char tmp = (char) peek_c;
        peek_c = NONE;
        return tmp;
    }
    char c;
    char *line; // pointer to queue item
    do{
        xQueuePeek(ble_in_queue, &line, portMAX_DELAY);
        c = line[line_off];
        line_off++;
        if( '\0' == c ) {
            /* pop the element from the queue */
            xQueueReceive(ble_in_queue, &line, portMAX_DELAY);
            line_off = 0;
        }
    }while('\0' == c);

    return c;
}

/* When we peek ahead to handle \r\n */
static void ble_return_char(int fd, int c){
    peek_c = c;
}

static ssize_t ble_read(int fd, void* data, size_t size) {
	char *data_c = (char *)data;

    _lock_acquire_recursive(&s_ble_read_lock);

    size_t received = 0;
    while(received < size){
        int c = ble_read_char(fd);

        ESP_LOGD(TAG, "Char: %02X; Off: %d", (char) c, received);
        if ( '\r' == (char)c ) {
            if (s_rx_mode == ESP_LINE_ENDINGS_CR) {
                // default
                c = '\n';
            } else if (s_rx_mode == ESP_LINE_ENDINGS_CRLF) {
                /* look ahead */
                int c2 = ble_read_char(fd);
                if (c2 == NONE) {
                    /* could not look ahead, put the current character back */
                    ble_return_char(fd, (char)c);
                    break;
                }
                if (c2 == '\n') {
                    /* this was \r\n sequence. discard \r, return \n */
                    c = '\n';
                } else {
                    /* \r followed by something else. put the second char back,
                     * it will be processed on next iteration. return \r now.
                     */
                    ble_return_char(fd, (char)c2);
                }
            }
        } else if (c == NONE) {
            break;
        }

        data_c[received] = (char) c;
        ++received;

        if ( '\n' == (char)c) {
            break;
        }
    }

    _lock_release_recursive(&s_ble_read_lock);

#if 0
    {
        char buf[100];
        sprintf(buf, "ble_read returning %d\n", received);
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
    }
#endif
    if(received > 0){
        return received;
    }
    errno = EWOULDBLOCK;
    return -1;
}

static int ble_fstat(int fd, struct stat * st) {
    assert(fd == 0);
    st->st_mode = S_IFCHR;
    return 0;
}

static int ble_close(int fd){
    /* Dummy since you don't really close this */
    assert(fd == 0);
    return 0;
}


#define CONFIG_JOLT_BLE_SPP_SELECT_EN 0
#if CONFIG_JOLT_BLE_SPP_SELECT_EN
/* Stuff for select() */
static SemaphoreHandle_t *_signal_sem = NULL;
static fd_set *_readfds = NULL;
static fd_set *_writefds = NULL;
static fd_set *_errorfds = NULL;
static fd_set *_readfds_orig = NULL;
static fd_set *_writefds_orig = NULL;
static fd_set *_errorfds_orig = NULL;

static _lock_t s_one_select_lock;

static portMUX_TYPE ble_selectlock = portMUX_INITIALIZER_UNLOCKED;
static void ble_end_select();

static portMUX_TYPE *ble_get_selectlock() {
    return &ble_selectlock;
}

/* SELECT notes
 *
 */
static void select_notif_callback(uart_port_t uart_num, uart_select_notif_t uart_select_notif, BaseType_t *task_woken)
{
    switch (uart_select_notif) {
        case UART_SELECT_READ_NOTIF:
            if (FD_ISSET(uart_num, _readfds_orig)) {
                FD_SET(uart_num, _readfds);
                esp_vfs_select_triggered_isr(_signal_sem, task_woken);
            }
            break;
        case UART_SELECT_WRITE_NOTIF:
            if (FD_ISSET(uart_num, _writefds_orig)) {
                FD_SET(uart_num, _writefds);
                esp_vfs_select_triggered_isr(_signal_sem, task_woken);
            }
            break;
        case UART_SELECT_ERROR_NOTIF:
            if (FD_ISSET(uart_num, _errorfds_orig)) {
                FD_SET(uart_num, _errorfds);
                esp_vfs_select_triggered_isr(_signal_sem, task_woken);
            }
            break;
    }
}

static esp_err_t ble_start_select(int nfds, fd_set *readfds, fd_set *writefds,
        fd_set *exceptfds, SemaphoreHandle_t *signal_sem){
    /* Setting up the environment for detection of read/write/error conditions 
     * on file descriptors belonging to BLE VFS.
     */

    if (_lock_try_acquire(&s_one_select_lock)) {
        return ESP_ERR_INVALID_STATE;
    }
    portENTER_CRITICAL( ble_get_selectlock() );

    #define FD_ERR_CHECK(x, y) \
        if( x ) {\
            portEXIT_CRITICAL(ble_get_selectlock());\
            ble_end_select(); \
            return y; \
        }
    #define FD_MEM_ALLOC(x, y) \
        FD_ERR_CHECK( NULL == (x = malloc(sizeof(fd_set))), ESP_ERR_NO_MEM );

    FD_ERR_CHECK(_readfds || _writefds || _errorfds ||
            _readfds_orig || _writefds_orig || _errorfds_orig ||
            _signal_sem,
            ESP_ERR_INVALID_STATE);

    FD_MEM_ALLOC(_readfds_orig);
    FD_MEM_ALLOC(_writeds_orig);
    FD_MEM_ALLOC(_errorfds_orig);

    #undef FD_ERR_CHECK
    #undef FD_MEM_ALLOC

    /* Set Callbacks for all file descriptors*/
    for (int i = 0; i < nfds; ++i) {
        if (FD_ISSET(i, readfds) || FD_ISSET(i, writefds) || FD_ISSET(i, exceptfds)) {
            ble_set_select_notif_callback(i, select_notif_callback);
        }
    }

    _signal_sem = signal_sem;

    _readfds = readfds;
    _writefds = writefds;
    _errorfds = exceptfds;

    *_readfds_orig = *readfds;
    *_writefds_orig = *writefds;
    *_errorfds_orig = *exceptfds;

	FD_ZERO(readfds);
    FD_ZERO(writefds);
    FD_ZERO(exceptfds);

    // todo; more stuff here

    return ESP_OK;
}

static void ble_end_select() {
    portENTER_CRITICAL(ble_get_selectlock());
    for (int i = 0; i < ble_NUM; ++i) {
        ble_set_select_notif_callback(i, NULL);
    }

    _signal_sem = NULL;

    _readfds = NULL;
    _writefds = NULL;
    _errorfds = NULL;

    if (_readfds_orig) {
        free(_readfds_orig);
        _readfds_orig = NULL;
    }

    if (_writefds_orig) {
        free(_writefds_orig);
        _writefds_orig = NULL;
    }

    if (_errorfds_orig) {
        free(_errorfds_orig);
        _errorfds_orig = NULL;
    }
    portEXIT_CRITICAL(ble_get_selectlock());
    _lock_release(&s_one_select_lock);    portENTER_CRITICAL(ble_get_selectlock());
}
#endif

static void esp_vfs_dev_ble_spp_register() {
    esp_vfs_t vfs = {
        .flags = ESP_VFS_FLAG_DEFAULT,
        .write = &ble_write,
        .open = &ble_open,
        .fstat = &ble_fstat,
        .close = &ble_close,
        .read = &ble_read,
#if CONFIG_JOLT_BLE_SPP_SELECT_EN
        .start_select = &ble_start_select,
        .end_select = &ble_end_select,
#endif
    };
    ESP_ERROR_CHECK(esp_vfs_register("/dev/ble", &vfs, NULL));
}
/* END DRIVER STUFF */

static void spp_cmd_task(void * arg) {
    esp_vfs_dev_ble_spp_register();
    ble_stdin  = fopen("/dev/ble/0", "r");
    ble_stdout = fopen("/dev/ble/0", "w");
    ble_stderr = fopen("/dev/ble/0", "w");

    char *buf = NULL;
    for(;;){
        if ( NULL == buf ) {
            buf = calloc(1, CONFIG_JOLT_CONSOLE_MAX_CMD_LEN);
        }
        //ESP_LOGI(TAG, "spp_cmd_task buf pointer: %p", buf);
        char *ptr = buf;
        uint16_t i;
        for(i=0; i<CONFIG_JOLT_CONSOLE_MAX_CMD_LEN; i++, ptr++){
            fread(ptr, 1, 1, ble_stdin);
            if('\n' == *ptr){
                *ptr = '\0';
                break;
            }
        }
        if(i>0){
            jolt_cmd_process(buf, ble_stdin, ble_stdout, ble_stderr, false);
            buf = NULL;
        }
    }

    vTaskDelete(NULL);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    esp_err_t err;
    ESP_LOGE(GATTS_TABLE_TAG, "GAP_EVT, event %d\n", event);

    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising( (esp_ble_adv_params_t *)&spp_adv_params );
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        /* advertising start complete event to indicate advertising 
         * start successfully or failed. */
        if((err = param->adv_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TABLE_TAG, "Advertising start failed: %s\n", 
                    esp_err_to_name(err));
        }
        break;
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    ESP_LOGI(GATTS_TABLE_TAG, "EVT %d, gatts if %d\n", event, gatts_if);

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        /* param contains status and application id to register */
        if (param->reg.status == ESP_GATT_OK) {
            spp_profile_tab[SPP_PROFILE_A_APP_ID].gatts_if = gatts_if;
        } 
        else {
            ESP_LOGI(GATTS_TABLE_TAG, 
                    "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id, param->reg.status);
            return;
        }
    }

    /* Call the gatts_cb for the speicified gatts_if */
    for (uint16_t idx = 0; idx < SPP_PROFILE_NUM; idx++) {
        /* ESP_GATT_IF_NONE, not specify a certain gatt_if,
         * need to call every profile cb function */
        if (gatts_if == ESP_GATT_IF_NONE || gatts_if == spp_profile_tab[idx].gatts_if) {
            if ( NULL != spp_profile_tab[idx].gatts_cb) {
                spp_profile_tab[idx].gatts_cb(event, gatts_if, param);
            }
        }
    }
}

/* To be called during Jolt startup
 * Initialized/Registers all bluetooth related hardware/software */
void jolt_bluetooth_setup() {
    esp_err_t ret;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed, error code = %x\n",
                __func__, ret);
        return;
    }
    else {
        ESP_LOGI(TAG, "[bt] Initialized Controller");
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed, error code = %x\n",
                __func__, ret);
        return;
    }
    else {
        ESP_LOGI(TAG, "[bt] Enabled Controller");
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed, error code = %x\n",
                __func__, ret);
        return;
    }
    else {
        ESP_LOGI(TAG, "[bt] Initialized Bluedroid");
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s\n",
                __func__, esp_err_to_name(ret));
        return;
    }
    else {
        ESP_LOGI(TAG, "[bt] Enabled Bluedroid");
    }

    //register the  callback function to the gap module
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "%s gap register failed, error code = %x\n",
                __func__, ret);
        return;
    }
    else {
        ESP_LOGI(TAG, "[bt] Registered GAP callback");
    }


    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if(ret){
        ESP_LOGE(GATTS_TABLE_TAG, "%s gatts register failed, error code = %x\n",
                __func__, ret);
        return;
    }
    else {
        ESP_LOGI(TAG, "[bt] Registered GATTS callback");
    }

    ret = esp_ble_gatts_app_register(SPP_PROFILE_A_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "%s gatts app register failed, error code = %x\n",
                __func__, ret);
    }
    else {
        ESP_LOGI(TAG, "[bt] Registered GATTS App");
    }

    ble_in_queue = xQueueCreate(10, sizeof(char *));
    xTaskCreate(&spp_cmd_task, "spp_cmd_task", 4096, NULL, 10, NULL);

    ESP_LOGI(TAG, "Done setting up bluetooth.");

}
#else

/* Stubs */
void jolt_bluetooth_setup(){
    return;
}

#endif
