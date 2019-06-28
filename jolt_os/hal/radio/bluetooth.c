/* What works:
 *    *streams: ble_stdin, ble_stdout, ble_stderr
 * What doesn't work:
 *    *select
*/
//#define LOG_LOCAL_LEVEL 4

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

#include "hal/radio/spp_recv_buf.h"
#include "linenoise/linenoise.h"
#include "bluetooth.h"
#include "bluetooth_state.h"

#include "jolt_gui/jolt_gui.h"

#define GATTS_SEND_REQUIRE_CONFIRM false

FILE *ble_stdin;
FILE *ble_stdout;
FILE *ble_stderr;

static void gap_event_handler(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *);
static void gatts_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

static const char TAG[] = "bluetooth.c";

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
    size_t remaining = size;
    const char *data_c = (const char *)data;
    _lock_acquire_recursive(&s_ble_write_lock);

#if LOG_LOCAL_LEVEL >= 4 /* debug */
        {
            char buf[100] = { 0 };
            sprintf(buf, "%s write %d bytes\n", __func__, size);
            uart_write_bytes(UART_NUM_0, buf, strlen(buf));
        }
#endif

    int idx = 0;
    do{
        esp_err_t res;
        size_t print_len = remaining;
        if( print_len > 512 ) {
            print_len = 512;
        }

#if LOG_LOCAL_LEVEL >= 4 /* debug */
        {
            char buf[100] = { 0 };
            sprintf(buf, "Sending %d bytes: ", print_len);
            uart_write_bytes(UART_NUM_0, buf, strlen(buf));
            uart_write_bytes(UART_NUM_0, &data_c[idx], print_len);
            uart_write_bytes(UART_NUM_0, "\n", 1);
        }
#endif

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
        remaining -= print_len;
    } while(remaining>0);

    _lock_release_recursive(&s_ble_write_lock);
    return size;
}

static int peek_c = NONE;
int ble_read_char(int fd, TickType_t timeout) {
    static uint32_t line_off = 0; // offset into most recent queue item
    if ( peek_c != NONE ){
        char tmp = (char) peek_c;
        peek_c = NONE;
        return tmp;
    }
    char c;
    ble_packet_t packet = { 0 };

    if(!xQueuePeek(ble_in_queue, &packet, timeout)) return NONE;
    c =packet.data[line_off];
    line_off++;
    
#if LOG_LOCAL_LEVEL >= 4 /* debug */
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "line_off: %d\n", line_off);
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
        snprintf(buf, sizeof(buf), "packet.len: %d\n\n", packet.len);
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
    }
#endif

    if( line_off == packet.len ) {
        /* pop the element from the queue */
        xQueueReceive(ble_in_queue, &packet, portMAX_DELAY);
        free(packet.data);
        line_off = 0;
    }

    return c;
}

/* When we peek ahead to handle \r\n */
static void ble_return_char(int fd, int c) {
    peek_c = c;
}

static ssize_t ble_read(int fd, void* data, size_t size) {
    return ble_read_timeout(fd, data, size, portMAX_DELAY);
}

ssize_t ble_read_timeout(int fd, void* data, size_t size, TickType_t timeout) {
    char *data_c = (char *)data;

    _lock_acquire_recursive(&s_ble_read_lock);
#if LOG_LOCAL_LEVEL >= 4 /* debug */
    {
        const char buf[] = "ble_read_lock acquired\n";
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
    }
#endif

    size_t received = 0;
    while(received < size){
        int c = ble_read_char(fd, timeout);

        ESP_LOGD(TAG, "Char: %02X; Off: %d", (char) c, received);
        if ( '\r' == (char)c ) {
            if (s_rx_mode == ESP_LINE_ENDINGS_CR) {
                // default
                c = '\n';
            } else if (s_rx_mode == ESP_LINE_ENDINGS_CRLF) {
                /* look ahead */
                int c2 = ble_read_char(fd, portMAX_DELAY);
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

#if LOG_LOCAL_LEVEL >= 4 /* debug */
    {
        const char buf[] = "ble_read_lock releasing\n";
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
    }
#endif

    _lock_release_recursive(&s_ble_read_lock);

#if LOG_LOCAL_LEVEL >= 4 /* debug */
    if(received > 0){
        /* Display what was received */
        char buf[100];
        sprintf(buf, "ble_read returning %d bytes\nreceived message: ", received);
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
        uart_write_bytes(UART_NUM_0, data_c, received-1);
        uart_write_bytes(UART_NUM_0, "\n", 1);
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
        fd_set *exceptfds, SemaphoreHandle_t *signal_sem) {
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

void esp_vfs_dev_ble_spp_register() {
    if ( NULL == ble_in_queue ) {
        ble_in_queue = xQueueCreate(10, sizeof(ble_packet_t));
    }

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

static void add_all_bonded_to_whitelist() {
    int dev_num = esp_ble_get_bond_device_num();
    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    for (int i = 0; i < dev_num; i++) {
        esp_ble_gap_update_whitelist(JOLT_BLE_WHITELIST_ADD, dev_list[i].bd_addr, JOLT_BLE_WHITELIST_ADDR_TYPE);
    }

    free(dev_list);
}

static const char* gap_evt_to_str(esp_gap_ble_cb_event_t event) {
#define CASE(x) case x: return #x;
    switch(event) {
        CASE(ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SCAN_RESULT_EVT);
        CASE(ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_ADV_START_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SCAN_START_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_AUTH_CMPL_EVT);
        CASE(ESP_GAP_BLE_KEY_EVT);
        CASE(ESP_GAP_BLE_SEC_REQ_EVT);
        CASE(ESP_GAP_BLE_PASSKEY_NOTIF_EVT);
        CASE(ESP_GAP_BLE_PASSKEY_REQ_EVT);
        CASE(ESP_GAP_BLE_OOB_REQ_EVT);
        CASE(ESP_GAP_BLE_LOCAL_IR_EVT);
        CASE(ESP_GAP_BLE_LOCAL_ER_EVT);
        CASE(ESP_GAP_BLE_NC_REQ_EVT);
        CASE(ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SET_STATIC_RAND_ADDR_EVT);
        CASE(ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT);
        CASE(ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT);
        CASE(ESP_GAP_BLE_UPDATE_DUPLICATE_EXCEPTIONAL_LIST_COMPLETE_EVT);
        default:
            return "<unknown>";
    }
#undef CASE
}

static const char* key_type_to_str( esp_ble_key_type_t key ) {
#define CASE(x) case x: return #x;
    switch( key ) {
        case ESP_LE_KEY_NONE:  return "None";
        case ESP_LE_KEY_PENC:  return "Peer Encryption Key";
        case ESP_LE_KEY_PID:   return "Peer Identity Key";
        case ESP_LE_KEY_PCSRK: return "Peer SRK";
        case ESP_LE_KEY_PLK:   return "Peer Link Key";
        case ESP_LE_KEY_LLK:   return "Local Link Key";
        case ESP_LE_KEY_LENC:  return "Local Encryption Key";
        case ESP_LE_KEY_LID:   return "Local Identity Key";
        case ESP_LE_KEY_LCSRK: return "Local CSRK has been delivered to peer";
        default:
            return "<unknown>";
    }
#undef CASE
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {

    esp_err_t err;
    ESP_LOGI(TAG, "GAP event %d: %s", event, gap_evt_to_str(event));
    switch (event) {
        /***************
         * Advertising *
         ***************/
        /* Triggered by: esp_ble_gap_config_adv_data_raw( raw_data, raw_data_len ) */
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT: /* fall through */
        /* Triggered by: esp_ble_gap_config_adv_data( &adv_data ) */
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            /* Start Advertising To Whitelisted Devices */
            ESP_ERROR_CHECK(esp_ble_gap_start_advertising( 
                    (esp_ble_adv_params_t *)&spp_adv_wht_params ));
            break;

        /* esp_ble_gap_start_advertising */
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            /* advertising start complete event to indicate advertising 
             * start successfully or failed. */
            if((err = param->adv_start_cmpl.status) != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Advertising start failed: %s\n", 
                        esp_err_to_name(err));
            }
            break;

        /*********
         * Scans *
         *********/
        case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
            /* scan response data set complete */
            break;
        /* Triggered by: esp_ble_gap_config_scan_rsp_data_raw( raw_data, raw_data_len ) */
        case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
            break;

        /* Triggered by: esp_ble_gap_set_scan_params( &scan_params ) */
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            /* scan parameters set complete */
            break;
        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            /* one scan result ready */
			/* todo: add debug logging here */
            break;
        /* Triggered by: esp_ble_gap_start_scanning( duration ) */
        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            /* Not used by Jolt*/
            break;
        /* Triggered by: esp_ble_gap_stop_scanning( ) */
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
			if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
				ESP_LOGE(TAG, "Scan stop failed, error status = %x", param->scan_stop_cmpl.status);
			}
            else {
                ESP_LOGI(TAG, "Stop scan successfully");
            }
            break;

        /************
         * Security *
         ************/
        case ESP_GAP_BLE_AUTH_CMPL_EVT: {
            /* Keys have been exchanged successfully between devices,
             * the pairing process is completed and encryption of payload data can be started.
             */

            /* [logging] Print Connection Data */
#if LOG_LOCAL_LEVEL >= 3 /* info */
            {
                esp_bd_addr_t bd_addr;
                memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr,
                        sizeof(esp_bd_addr_t));
                ESP_LOGI(TAG, "remote BD_ADDR: %08x%04x",
                        (bd_addr[0] << 24) + (bd_addr[1] << 16)
                        + (bd_addr[2] << 8) + bd_addr[3], (bd_addr[4] << 8)
                        + bd_addr[5]);
            }
#endif
            ESP_LOGI(TAG, "address type = %d", 
                    param->ble_security.auth_cmpl.addr_type);
            ESP_LOGI(TAG, "pair status = %s", 
                    param->ble_security.auth_cmpl.success ? "success" : "fail");

			if (param->ble_security.auth_cmpl.success) {
				ESP_LOGI(TAG, "auth mode = 0x%x", 
                        param->ble_security.auth_cmpl.auth_mode);    
                /* Add the address to the whitelist */
                esp_err_t err;
                esp_bd_addr_t bd_addr;
                memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr,
                        sizeof(esp_bd_addr_t));
                err = esp_ble_gap_update_whitelist(JOLT_BLE_WHITELIST_ADD, bd_addr, JOLT_BLE_WHITELIST_ADDR_TYPE);
                if( ESP_OK != err ){
                    ESP_LOGE(TAG, "Failed to add bd_addr to whitelist");
                }
			}
            else {
				ESP_LOGI(TAG, "fail reason = 0x%x", 
                        param->ble_security.auth_cmpl.fail_reason);
			}
            break;
        }
        case ESP_GAP_BLE_KEY_EVT:
            /* Triggered for every key exchange message */
            ESP_LOGI(TAG, "key type 0x%02x: %s", param->ble_security.ble_key.key_type,
                    key_type_to_str(param->ble_security.ble_key.key_type));
            break;
        case ESP_GAP_BLE_SEC_REQ_EVT:
            /* Slave requests to start encryption.
             * Send the [true] security response to the peer device to accept the security request.
             * To reject the security request, send the security response with [false] value*/
            ESP_LOGI(TAG, "Slave requesting security.");
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        case ESP_GAP_BLE_PASSKEY_REQ_EVT:
            /* If this device has digit input capabilities, and the remote device has
             * display capabilities, this is triggered to input the digits shown on
             * the remote display using esp_ble_passkey_reply() */
            /* Not used in Jolt */
            break;
        case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:
            /* The app will receive this evt when the IO has Output capability 
             * and the peer device IO has Input capability.
             * Show the passkey integer param->ble_security.key_notif.passkey 
             * to the user to input it in the peer device. */
            ESP_LOGI(TAG, "ESP_GAP_BLE_PASSKEY_NOTIF_EVT, the passkey Notify number:%06d",
                    param->ble_security.key_notif.passkey);
            /* See bluetooth_pair.c for gui handling */
            break;
        case ESP_GAP_BLE_NC_REQ_EVT:
            /* The app will receive this evt when the IO has DisplayYesNO 
             * capability and the peer device IO also has DisplayYesNo capability.
             * Show the passkey integer param->ble_security.key_notif.passkey
             * to the user to confirm it with the number displayed by peer deivce. */
            /* Not used in Jolt */
            /* This shouldn't trigger due to the IO_CAP config */
            break;
        /* Triggered by: esp_ble_gap_update_whitelist() */
        case ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT:
            break;
        case ESP_GAP_BLE_OOB_REQ_EVT:
            /* Out of Band is currently not supported by Jolt.
             * Support was recently added to ESP-IDF*/
            break;

        /**********************
         * Security - Bonding *
         **********************/
        case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
            ESP_LOGD(TAG,
                    "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d",
                    param->remove_bond_dev_cmpl.status);
            ESP_LOGI(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV");
            ESP_LOGI(TAG, "-----ESP_GAP_BLE_REMOVE_BOND_DEV----");
            esp_log_buffer_hex(TAG, (void *)param->remove_bond_dev_cmpl.bd_addr,
                    sizeof(esp_bd_addr_t));
            ESP_LOGI(TAG, "------------------------------------");
            break;
        case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:
            break;
        case ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT:
            break;

        /************************************
         * General Parameter Configurations *
         ************************************/
        /* Triggered by: esp_ble_gap_update_conn_params( &params ) */
        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            break;
        /* Triggered by: esp_ble_gap_set_pkt_data_len() */
        case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:
            break;
        /* Triggered by: esp_ble_gap_config_local_privacy() */
        case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT: {
            if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS){
                ESP_LOGE(TAG, "config local privacy failed, error status = %x",
                        param->local_privacy_cmpl.status);
                break;
            }
        }

        /****************
         * Meta Queries *
         ****************/
        case ESP_GAP_BLE_READ_RSSI_COMPLETE_EVT:
            break;

        /********
         * Misc *
         ********/
        /* Triggered by: sp_ble_gap_add_duplicate_scan_exceptional_device() */
        case ESP_GAP_BLE_UPDATE_DUPLICATE_EXCEPTIONAL_LIST_COMPLETE_EVT:
            break;

        /* The following events were used for internal esp-idf development; deprecated:
         *     ESP_GAP_BLE_LOCAL_IR_EVT
         *     ESP_GAP_BLE_LOCAL_ER_EVT
         */

        default:
            break;
    }
    jolt_gui_gap_cb(event, param);
}

static void gatts_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        /* param contains status and application id to register */
        if (param->reg.status == ESP_GATT_OK) {
            spp_profile_tab[SPP_PROFILE_A_APP_ID].gatts_if = gatts_if;
        } 
        else {
            ESP_LOGI(TAG, 
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

void jolt_bluetooth_config_security(bool bond) {
    /* This section sets the security parameters in the enumerated order */
    /* ESP_BLE_SM_PASSKEY seems to be undocumented*/
    {
        /* Secure Connections with MITM Protection and Bonding */
        esp_ble_auth_req_t auth_req;
        auth_req = bond ? ESP_LE_AUTH_REQ_SC_MITM_BOND : ESP_LE_AUTH_REQ_SC_MITM;
        ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t)));
    }

    {
        /* Register Jolt's IO capability */
        /* Set as CAP_OUT so that you MUST enter code on computer/smartphone */
        esp_ble_io_cap_t iocap = ESP_IO_CAP_OUT;
        ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t)));
    }

    {
        /* Initiator Key Distribution/Generation */
        /* Type of key the smartphone/computer should distribute to Jolt */
        uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
        ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t)));

        /* Type of key Jolt can distribute to smartphone/computer */
        uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
        ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t)));
    }

    {
        /* Set Bluetooth Encryption Keysize */
        uint8_t key_size = 16;      /* 128-bit key */
        ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t)));
    }

    /* ESP_BLE_SM_SET_STATIC_PASSKEY and ESP_BLE_SM_CLEAR_STATIC_PASSKEY are not used */

    {
        uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_ENABLE; // todo: maybe set this to ENABLE
        ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t)));
    }

    {
        /* ESP_BLE_SM_OOB_SUPPORT is not fully fleshed out in ESP32 */
        uint8_t oob_support = ESP_BLE_OOB_DISABLE;
        ESP_ERROR_CHECK(esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t)));
    }
}

esp_err_t jolt_bluetooth_start() {
    esp_err_t err = ESP_OK;

    /* Create BT Controller */
    {
        esp_bt_controller_config_t cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        esp_bt_controller_status_t status;
        status = esp_bt_controller_get_status();
        switch(status) {
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_IDLE:
                err = esp_bt_controller_init( &cfg );
                if( err ) {
                    ESP_LOGE(TAG, "%s bt_controller_init failed, "
                            "error code = %s\n", __func__, esp_err_to_name(err));
                    goto exit;
                }
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_INITED:
                err = esp_bt_controller_enable(ESP_BT_MODE_BLE);
                if( err ) {
                    ESP_LOGE(TAG, "%s bt_controller_enable failed, "
                            "error code = %s\n", __func__, esp_err_to_name(err));
                    goto exit;
                }
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_ENABLED:
                /* do nothing */
            default:
                /* do nothing */
                break;
        }
    }

    /* Create Bluedroid */
    {
        esp_bluedroid_status_t status;
        status = esp_bluedroid_get_status();
        switch(status){
            /* Falls through */
            case ESP_BLUEDROID_STATUS_UNINITIALIZED:
                err = esp_bluedroid_init();
                if ( err ) {
                    ESP_LOGE(TAG, "%s init bluedroid failed, error code = %s\n",
                            __func__, esp_err_to_name(err));
                    goto exit;
                }
            /* Falls through */
            case ESP_BLUEDROID_STATUS_INITIALIZED:
                err = esp_bluedroid_enable();
                if ( err ) {
                    ESP_LOGE(TAG, "%s enable bluedroid failed: %s\n",
                            __func__, esp_err_to_name(err));
                    goto exit;
                }
            /* Falls through */
            case ESP_BLUEDROID_STATUS_ENABLED:
                /* do nothing */
            default:
                /* do nothing */
                break;
        }
    }

    add_all_bonded_to_whitelist();

    err = esp_ble_gap_register_callback(gap_event_handler);
    if ( err ){
        ESP_LOGE(TAG, "%s gap register failed, error code = %s\n",
                __func__, esp_err_to_name(err));
        goto exit;
    }

    err = esp_ble_gatts_register_callback(gatts_event_handler);
    if( err ){
        ESP_LOGE(TAG, "%s gatts register failed, error code = %s\n",
                __func__, esp_err_to_name(err) );
        goto exit;
    }

    err = esp_ble_gatts_app_register(SPP_PROFILE_A_APP_ID);
    if ( err ){
        ESP_LOGE(TAG, "%s gatts app register failed, error code = %s\n",
                __func__, esp_err_to_name(err) );
        goto exit;
    }

    /* Configure Bluetooth Security Parameters */
    jolt_bluetooth_config_security( true );


exit:
    return ESP_OK;
}

esp_err_t jolt_bluetooth_stop() {
    esp_err_t err = ESP_OK;

    /* Destroy Bluedroid */
    {
        esp_bluedroid_status_t status;
        status = esp_bluedroid_get_status();
        switch(status){
            /* Falls through */
            case ESP_BLUEDROID_STATUS_ENABLED:
                err = esp_bluedroid_disable();
                if ( ESP_OK != err ){
                    ESP_LOGE(TAG, "%s bluedroid disable failed, "
                            "error code = %s\n", __func__, esp_err_to_name(err) );
                    goto exit;
                }
            /* Falls through */
            case ESP_BLUEDROID_STATUS_INITIALIZED:
                err = esp_bluedroid_deinit();
                if ( ESP_OK != err ){
                    ESP_LOGE(TAG, "%s bluedroid deinit failed, "
                            "error code = %s\n",
                            __func__, esp_err_to_name(err) );
                    goto exit;
                }
            /* Falls through */
            case ESP_BLUEDROID_STATUS_UNINITIALIZED:
                /* do nothing */
            default:
                /* do nothing */
                break;
        }
    }

    /* Destroy BT Controller */
    {
        esp_bt_controller_status_t status;
        status = esp_bt_controller_get_status();
        switch(status) {
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_ENABLED:
                err = esp_bt_controller_disable();
                if ( ESP_OK != err ){
                    ESP_LOGE(TAG, "%s bt controller disable failed, "
                            "error code = %s\n", __func__, esp_err_to_name(err) );
                    goto exit;
                }
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_INITED:
                err = esp_bt_controller_deinit();
                if ( ESP_OK != err ) {
                    ESP_LOGE(TAG, "%s bt controller deinit failed, "
                            "error code = %s\n", __func__, esp_err_to_name(err) );
                    goto exit;
                }
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_IDLE:
                /* do nothing */
            default:
                /* do nothing */
                break;
        }
    }
exit:
    return err;
}

/* Start Advertising to ALL devices; useful for pairing */
esp_err_t jolt_bluetooth_adv_all_start() {
    esp_err_t err;
    jolt_bluetooth_adv_stop();
    
    err = esp_ble_gap_start_advertising( (esp_ble_adv_params_t *)&spp_adv_pair_params );
    if( ESP_OK != err ){
        ESP_LOGE(TAG, "Failed to start BT advertising: %d", err);
        return err;
    }

    return ESP_OK;
}

/* Start Advertising to whitelisted devices */
esp_err_t jolt_bluetooth_adv_wht_start() {
    esp_err_t err;
    jolt_bluetooth_adv_stop();
    
    err = esp_ble_gap_start_advertising( (esp_ble_adv_params_t *)&spp_adv_wht_params );
    if( ESP_OK != err ){
        ESP_LOGE(TAG, "Failed to start BT advertising: %d", err);
        return err;
    }

    return ESP_OK;
}

/* Stop advertising */
esp_err_t jolt_bluetooth_adv_stop() {
    return esp_ble_gap_stop_advertising();
}

#else

/* Stubs */
esp_err_t jolt_bluetooth_start(){
    return ESP_OK;
}

esp_err_t jolt_bluetooth_stop(){
    return ESP_OK;
}

#endif
