/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
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

#include "jolt_globals.h"
#include "esp_console.h"

#include "spp_recv_buf.h"
#include "linenoise/linenoise.h"
#include "bluetooth.h"


#define spp_sprintf(s,...)         sprintf((char*)(s), ##__VA_ARGS__)
#define SPP_DATA_MAX_LEN           (512)
#define SPP_CMD_MAX_LEN            (20)
#define SPP_STATUS_MAX_LEN         (20)
#define SPP_DATA_BUFF_MAX_LEN      (2*1024)

#define GATTS_SEND_REQUIRE_CONFIRM false

FILE *ble_stdin;
FILE *ble_stdout;
FILE *ble_stderr;

static uint8_t find_char_and_desr_index(uint16_t);
static void gap_event_handler(esp_gap_ble_cb_event_t, esp_ble_gap_cb_param_t *);
static void gatts_profile_event_handler(esp_gatts_cb_event_t, 
        esp_gatt_if_t, esp_ble_gatts_cb_param_t *);
static void gatts_event_handler(esp_gatts_cb_event_t , 
        esp_gatt_if_t, esp_ble_gatts_cb_param_t *);

///Attributes State Machine
enum{
    SPP_IDX_SVC = 0,

    SPP_IDX_SPP_DATA_RECV_CHAR,
    SPP_IDX_SPP_DATA_RECV_VAL,

    SPP_IDX_SPP_DATA_NOTIFY_CHAR,
    SPP_IDX_SPP_DATA_NTY_VAL,
    SPP_IDX_SPP_DATA_NTF_CFG,

    SPP_IDX_SPP_COMMAND_CHAR,
    SPP_IDX_SPP_COMMAND_VAL,

    SPP_IDX_SPP_STATUS_CHAR,
    SPP_IDX_SPP_STATUS_VAL,
    SPP_IDX_SPP_STATUS_CFG,

    SPP_IDX_NB,
};

enum KEY_ACTION{
	KEY_NULL = 0,	    /* NULL */
	CTRL_A = 1,         /* Ctrl+a */
	CTRL_B = 2,         /* Ctrl-b */
	CTRL_C = 3,         /* Ctrl-c */
	CTRL_D = 4,         /* Ctrl-d */
	CTRL_E = 5,         /* Ctrl-e */
	CTRL_F = 6,         /* Ctrl-f */
	CTRL_H = 8,         /* Ctrl-h */
	TAB = 9,            /* Tab */
	CTRL_K = 11,        /* Ctrl+k */
	CTRL_L = 12,        /* Ctrl+l */
	ENTER = 10,         /* Enter (\n) */
	CTRL_N = 14,        /* Ctrl-n */
	CTRL_P = 16,        /* Ctrl-p */
	CTRL_T = 20,        /* Ctrl-t */
	CTRL_U = 21,        /* Ctrl+u */
	CTRL_W = 23,        /* Ctrl+w */
	ESC = 27,           /* Escape */
	BACKSPACE =  127    /* Backspace */
};


#define GATTS_TABLE_TAG  "GATTS_SPP_DEMO"
#define TAG  "GATTS_SPP_DEMO"

#define SPP_PROFILE_NUM             1
#define SPP_PROFILE_APP_IDX         0
#define ESP_SPP_APP_ID              0x56
#define SAMPLE_DEVICE_NAME          "Jolt"
#define SPP_SVC_INST_ID	            0

/// SPP Service
static const uint16_t spp_service_uuid = 0xFFE0;
/// Characteristic UUID
#define ESP_GATT_UUID_SPP_DATA_RECEIVE      0xFFE1 // smartphone->jolt
#define ESP_GATT_UUID_SPP_DATA_NOTIFY       0xFFE2
#define ESP_GATT_UUID_SPP_COMMAND_RECEIVE   0xABF3 // smartphone->jolt
#define ESP_GATT_UUID_SPP_COMMAND_NOTIFY    0xABF4

// Serial Port Profile Advertising Data
// esp_ble_adv_data_t
static const uint8_t spp_adv_data[23] = {
    0x02,0x01,0x06, // Flags
    0x03,0x03,0xE0,0xFF,
    0x0F,0x04,'J','o','l','t'
};

static uint16_t spp_mtu_size = 23;
static uint16_t spp_conn_id = 0xffff;
static esp_gatt_if_t spp_gatts_if = 0xff;
static xQueueHandle cmd_cmd_queue = NULL;

static bool enable_data_ntf = false;
static bool is_connected = false;
static esp_bd_addr_t spp_remote_bda = {0x0,};

static uint16_t spp_handle_table[SPP_IDX_NB];

/* Advertising Parameters */
static esp_ble_adv_params_t spp_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x40,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

/* Forward Declare Static Functions */
static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* Array to store each application profile; will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst spp_profile_tab[SPP_PROFILE_NUM] = {
    [SPP_PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

/***************************
 *  SPP PROFILE ATTRIBUTES *
 ***************************/

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))
static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint8_t char_prop_read_notify = ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE_NR|ESP_GATT_CHAR_PROP_BIT_READ;

/* SPP Service - data receive characteristic, read&write without response */
static const uint16_t spp_data_receive_uuid = ESP_GATT_UUID_SPP_DATA_RECEIVE;
static const uint8_t  spp_data_receive_val[20] = {0x00};

/* SPP Service - data notify characteristic, notify&read */
static const uint16_t spp_data_notify_uuid = ESP_GATT_UUID_SPP_DATA_NOTIFY;
static const uint8_t  spp_data_notify_val[20] = {0x00};
static const uint8_t  spp_data_notify_ccc[2] = {0x00, 0x00};

/* SPP Service - command characteristic, read&write without response */
static const uint16_t spp_command_uuid = ESP_GATT_UUID_SPP_COMMAND_RECEIVE;
static const uint8_t  spp_command_val[10] = {0x00};

/* SPP Service - status characteristic, notify&read */
static const uint16_t spp_status_uuid = ESP_GATT_UUID_SPP_COMMAND_NOTIFY;
static const uint8_t  spp_status_val[10] = {0x00};
static const uint8_t  spp_status_ccc[2] = {0x00, 0x00};

/* Full HRS Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t spp_gatt_db[SPP_IDX_NB] = {
    /* SPP -  Service Declaration */
    [SPP_IDX_SVC] = {
        .attr_control = {
            // response of R/W operation will be replied by GATT stack automatically
            .auto_rsp = ESP_GATT_AUTO_RSP
        },
        .att_desc = {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&primary_service_uuid, // UUID value
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = sizeof(spp_service_uuid),
            .length      = sizeof(spp_service_uuid),
            .value       = (uint8_t *)&spp_service_uuid
        }
    },

    //SPP -  data receive characteristic Declaration
    [SPP_IDX_SPP_DATA_RECV_CHAR] = {
        {
            ESP_GATT_AUTO_RSP
        },
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_declaration_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = CHAR_DECLARATION_SIZE,
            .length      = CHAR_DECLARATION_SIZE, 
            .value       = (uint8_t *)&char_prop_read_write
        }
    },

    //SPP -  data receive characteristic Value
    [SPP_IDX_SPP_DATA_RECV_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&spp_data_receive_uuid,
            .perm        = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .max_length  = SPP_DATA_MAX_LEN,
            .length      = sizeof(spp_data_receive_val),
            .value       = (uint8_t *)spp_data_receive_val
        }
    },

    //SPP -  data notify characteristic Declaration
    [SPP_IDX_SPP_DATA_NOTIFY_CHAR]  = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_declaration_uuid,
            .perm        = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
            .max_length  = CHAR_DECLARATION_SIZE,
            .length      = CHAR_DECLARATION_SIZE,
            .value       = (uint8_t *)&char_prop_read_notify
        }
    },

    //SPP -  data notify characteristic Value
    [SPP_IDX_SPP_DATA_NTY_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&spp_data_notify_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = SPP_DATA_MAX_LEN,
            .length      = sizeof(spp_data_notify_val),
            .value       = (uint8_t *)spp_data_notify_val
        }
    },

    //SPP -  data notify characteristic - Client Characteristic Configuration Descriptor
    [SPP_IDX_SPP_DATA_NTF_CFG] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_client_config_uuid,
            .perm        = ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
            .max_length  = sizeof(uint16_t),
            .length      = sizeof(spp_data_notify_ccc),
            .value       = (uint8_t *)spp_data_notify_ccc
        }
    },

    //SPP -  command characteristic Declaration
    [SPP_IDX_SPP_COMMAND_CHAR] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_declaration_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = CHAR_DECLARATION_SIZE,
            .length      = CHAR_DECLARATION_SIZE,
            .value       = (uint8_t *)&char_prop_read_write
        }
    },

    //SPP -  command characteristic Value
    [SPP_IDX_SPP_COMMAND_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {
           .uuid_length =  ESP_UUID_LEN_16,
           .uuid_p      =  (uint8_t *)&spp_command_uuid, 
           .perm        =  ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE, 
           .max_length  =  SPP_CMD_MAX_LEN,
           .length      =  sizeof(spp_command_val),
           .value       =  (uint8_t *)spp_command_val
        }
    },

    //SPP -  status characteristic Declaration
    [SPP_IDX_SPP_STATUS_CHAR] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_declaration_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = CHAR_DECLARATION_SIZE,
            .length      = CHAR_DECLARATION_SIZE, 
            .value       = (uint8_t *)&char_prop_read_notify
        }
    },

    //SPP -  status characteristic Value
    [SPP_IDX_SPP_STATUS_VAL] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&spp_status_uuid,
            .perm        = ESP_GATT_PERM_READ,
            .max_length  = SPP_STATUS_MAX_LEN,
            .length      = sizeof(spp_status_val),
            .value       = (uint8_t *)spp_status_val
        }
    },

    //SPP -  status characteristic - Client Characteristic Configuration Descriptor
    [SPP_IDX_SPP_STATUS_CFG] = {
        {ESP_GATT_AUTO_RSP},
        {
            .uuid_length = ESP_UUID_LEN_16,
            .uuid_p      = (uint8_t *)&character_client_config_uuid,
            .perm        = ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE,
            .max_length  = sizeof(uint16_t),
            .length      = sizeof(spp_status_ccc),
            .value       = (uint8_t *)spp_status_ccc
        }
    },
};



/* DRIVER STUFF */
static int     ble_open(const char * path, int flags, int mode );
static int     ble_fstat( int fd, struct stat * st );
static int     ble_close( int fd );
static ssize_t ble_write( int fd, const void * data, size_t size );
static ssize_t ble_read(  int fd, void* data, size_t size );


// Token signifying that no character is available
#define NONE -1

static esp_line_endings_t s_tx_mode =
#if CONFIG_NEWLIB_STDOUT_LINE_ENDING_CRLF
        ESP_LINE_ENDINGS_CRLF;
#elif CONFIG_NEWLIB_STDOUT_LINE_ENDING_CR
        ESP_LINE_ENDINGS_CR;
#else
        ESP_LINE_ENDINGS_LF;
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

// Basic functions for sending and receiving bytes over BLE
static void ble_tx_char(int fd, int c);
static int ble_rx_char(int fd);

// Functions for sending and receiving bytes which use BLE driver
static void ble_tx_char_via_driver(int fd, int c);
static int ble_rx_char_via_driver(int fd);

static _lock_t s_ble_read_lock;
static _lock_t s_ble_write_lock;
// One-character buffer used for newline conversion code
static int s_peek_char = NONE;

/* Lock ensuring that uart_select is used from only one task at the time */
static _lock_t s_one_select_lock;

/* Stuff for select() */
static SemaphoreHandle_t *_signal_sem = NULL;
static fd_set *_readfds = NULL;
static fd_set *_writefds = NULL;
static fd_set *_errorfds = NULL;
static fd_set *_readfds_orig = NULL;
static fd_set *_writefds_orig = NULL;
static fd_set *_errorfds_orig = NULL;

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
    esp_err_t res;
    do{
        uint16_t print_len = size;
        if( print_len > 512 ) {
            print_len = 512;
        }
        res = esp_ble_gatts_send_indicate(
                spp_gatts_if,
                spp_conn_id,
                spp_handle_table[SPP_IDX_SPP_DATA_NTY_VAL],
                print_len, (uint8_t*) &data_c[idx], GATTS_SEND_REQUIRE_CONFIRM);
        // todo: res error handling
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
    if( !xQueuePeek(cmd_cmd_queue, &line, portMAX_DELAY) ){
        /* no characters remaining */
        line_off = 0;
        return NONE;
    }
    else {
        const char x[] = "receiving_data ";
        uart_write_bytes(UART_NUM_0, 
                x, strlen(x) );
        //uart_write_bytes(UART_NUM_0, &c, 1);
        //uart_write_bytes(UART_NUM_0, "\n", 1);
    }
    c = line[line_off];
    line_off++;

    char tmp[10] = { 0 };
    sprintf(tmp, ": %02X\n", c);
    uart_write_bytes(UART_NUM_0, tmp, strlen(tmp));

    if( '\0' == c ) {
        /* pop the element from the queue */
        xQueueReceive(cmd_cmd_queue, &line, portMAX_DELAY);
        line_off = 0;
    }
    return c;
}

/* When we peek ahead to handle \r\n */
static void ble_return_char(int fd, int c){
    peek_c = c;
}

static ssize_t ble_read(int fd, void* data, size_t size) {
	char *data_c = (const char *)data;

    _lock_acquire_recursive(&s_ble_write_lock);

    size_t received = 0;
    while(received < size){
        int c = ble_read_char(fd);

        if (c == '\r') {
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

        if(c == '\0') {
            --received;
            break;
        }

        if (c == '\n') {
            break;
        }
    }

    _lock_release_recursive(&s_ble_write_lock);

    {
        char buf[100];
        sprintf(buf, "ble_read returning %d\n", received);
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
    }
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

void esp_vfs_dev_ble_spp_register() {
    esp_vfs_t vfs = {
        .flags = ESP_VFS_FLAG_DEFAULT,
        .write = &ble_write,
        .open = &ble_open,
        .fstat = &ble_fstat,
        .close = &ble_close,
        .read = &ble_read,
#if 0
        .start_select = &ble_start_select,
        .end_select = &ble_end_select,
#endif
    };
    ESP_ERROR_CHECK(esp_vfs_register("/dev/ble", &vfs, NULL));
}
/* END DRIVER STUFF */

static uint8_t find_char_and_desr_index(uint16_t handle) {
    for(int i = 0; i < SPP_IDX_NB ; i++) {
        if( handle == spp_handle_table[i]) {
            return i;
        }
    }
    return 0xff; // error
}

static void spp_cmd_task(void * arg) {
    uint8_t * cmd_id;
    char *line;

    
#if 0
#else
    esp_vfs_dev_ble_spp_register();
    ble_stdin  = fopen("/dev/ble/0", "r");
    ble_stdout = fopen("/dev/ble/0", "w");
    ble_stderr = fopen("/dev/ble/0", "w");
#endif
    //setlinebuf(ble_stdout); /* Flush buffer every \n */
    //setlinebuf(ble_stdin); /* Flush buffer every \n */
    stdin = ble_stdin;
    stdout = ble_stdout;
    stderr = ble_stderr;

    //setvbuf(stdin, NULL, _IONBF, 0);
    //setvbuf(stdout, NULL, _IONBF, 0);
    //setvbuf(stderr, NULL, _IONBF, 0);
    /*
    {
        char buf[100] = { 0 };
        int n = fread(buf, 1, sizeof(buf), stdin);
        const char x[] = "bloop\n";
        uart_write_bytes(UART_NUM_0, x, strlen(x));
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
        sprintf(buf, "Received %d bytes\n.", n);
        uart_write_bytes(UART_NUM_0, buf, strlen(buf));
        vTaskDelay(portMAX_DELAY);
    }
    */

    for(;;){
        char* line = linenoise("meow");
        uart_write_bytes(UART_NUM_0, line, strlen(line));
        printf("line: %s\n", line);
        //if(xQueueReceive(cmd_cmd_queue, &line, portMAX_DELAY)) {
            /* todo: refactor this, repeated code from console.c */
            if (line == NULL) { /* Ignore empty lines */
                continue;
            }
            
            /* Try to run the command */
            int ret;
            esp_err_t err = esp_console_run(line, &ret);
            if (err == ESP_ERR_NOT_FOUND) {
                // The command could be an app to run console commands from
                char *argv[CONFIG_JOLT_CONSOLE_MAX_ARGS + 1];
                printf("Unsuccessful command\n");
                /*
                // split_argv modifies line with NULL-terminators
                size_t argc = esp_console_split_argv(line, argv, sizeof(argv));
                if( launch_file(argv[0], "console", argc-1, argv+1) ) {
                    printf("Unsuccessful command\n");
                }
                */
            } else if (err == ESP_ERR_INVALID_ARG) {
                // command was empty
            } else if (err == ESP_OK && ret != ESP_OK) {
                printf("Command returned non-zero error code: 0x%x\n", ret);
            } else if (err != ESP_OK) {
                printf("Internal error: 0x%x\n", err);
            }

            free(line);
        //}
    }
    vTaskDelete(NULL);
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    esp_err_t err;
    ESP_LOGE(GATTS_TABLE_TAG, "GAP_EVT, event %d\n", event);

    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&spp_adv_params);
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

/* Only used to handle events for SPP_PROFILE_APP_IDX */
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    esp_ble_gatts_cb_param_t *p_data = (esp_ble_gatts_cb_param_t *) param;
    uint8_t res = 0xff;

    ESP_LOGI(GATTS_TABLE_TAG, "event = %x\n",event);
    switch (event) {
    	case ESP_GATTS_REG_EVT:
        	esp_ble_gap_set_device_name(SAMPLE_DEVICE_NAME);
        	esp_ble_gap_config_adv_data_raw((uint8_t *)spp_adv_data,
                    sizeof(spp_adv_data));
        	esp_ble_gatts_create_attr_tab(spp_gatt_db, gatts_if, 
                    SPP_IDX_NB, SPP_SVC_INST_ID);
       	    break;
    	case ESP_GATTS_READ_EVT:
            res = find_char_and_desr_index(p_data->read.handle);
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT : handle = %d\n", res);
            if(res == SPP_IDX_SPP_STATUS_VAL){
                //TODO: client read the status characteristic
            }
       	    break;
    	case ESP_GATTS_WRITE_EVT: {
    	    res = find_char_and_desr_index(p_data->write.handle);
            if(p_data->write.is_prep == false){
                ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_WRITE_EVT : handle = %d\n", res);
                if(res == SPP_IDX_SPP_COMMAND_VAL){
                    /* Allocate memory for 1 MTU;
                     * send it off to the cmd_cmd_queue */
                    ESP_LOGI(GATTS_TABLE_TAG, "SPP_IDX_SPP_COMMAND_VAL;"
                            " Allocating %d bytes.", spp_mtu_size-3);
                    uint8_t * spp_cmd_buff = NULL;
                    spp_cmd_buff = (uint8_t *)malloc((spp_mtu_size - 3) * sizeof(uint8_t));
                    if(spp_cmd_buff == NULL){
                        ESP_LOGE(GATTS_TABLE_TAG, "%s malloc failed\n", __func__);
                        break;
                    }
                    memset(spp_cmd_buff, 0, (spp_mtu_size - 3));
                    memcpy(spp_cmd_buff, p_data->write.value, p_data->write.len);
                    xQueueSend(cmd_cmd_queue, &spp_cmd_buff, 10/portTICK_PERIOD_MS);
                }
                else if(res == SPP_IDX_SPP_DATA_NTF_CFG){
                    ESP_LOGI(GATTS_TABLE_TAG, "SPP_IDX_SPP_DATA_NTF_CFG");
                    if( (p_data->write.len == 2)
                            &&(p_data->write.value[0] == 0x01)
                            &&(p_data->write.value[1] == 0x00) ) {
                        enable_data_ntf = true;
                    }
                    else if( (p_data->write.len == 2)
                            &&(p_data->write.value[0] == 0x00)
                            &&(p_data->write.value[1] == 0x00) ) {
                        enable_data_ntf = false;
                    }
                }
                else if( res == SPP_IDX_SPP_DATA_RECV_VAL ) {
                    /* Phone/Computer sent string to Jolt */
                    ESP_LOGI(GATTS_TABLE_TAG, "SPP_IDX_SPP_DATA_RECV_VAL");
                    #ifdef SPP_DEBUG_MODE
                    esp_log_buffer_char(GATTS_TABLE_TAG,
                            (char *)(p_data->write.value),p_data->write.len);
                    #else
                    uart_write_bytes(UART_NUM_0, 
                            (char *)(p_data->write.value), p_data->write.len);
                    uart_write_bytes(UART_NUM_0, "\n", 1);
                    #endif

                }
                else{
                    ESP_LOGI(GATTS_TABLE_TAG, "Unknown state machine attribute %d.",
                            res);
                    //TODO:
                }
            }
            else if( (p_data->write.is_prep == true)
                    && (res == SPP_IDX_SPP_DATA_RECV_VAL) ) {
                ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_PREP_WRITE_EVT : handle = %d\n", res);
                store_wr_buffer(p_data);
            }
      	 	break;
    	}
    	case ESP_GATTS_EXEC_WRITE_EVT:{
    	    ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT\n");
    	    if(p_data->exec_write.exec_write_flag){
    	        print_write_buffer();
    	        free_write_buffer();
    	    }
    	    break;
    	}
    	case ESP_GATTS_MTU_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVENT;"
                    "Setting MTU size to %d bytes.", p_data->mtu.mtu);
    	    spp_mtu_size = p_data->mtu.mtu;
    	    break;
    	case ESP_GATTS_CONF_EVT:
    	    break;
    	case ESP_GATTS_UNREG_EVT:
        	break;
    	case ESP_GATTS_DELETE_EVT:
        	break;
    	case ESP_GATTS_START_EVT:
        	break;
    	case ESP_GATTS_STOP_EVT:
        	break;
    	case ESP_GATTS_CONNECT_EVT:
    	    spp_conn_id = p_data->connect.conn_id;
    	    spp_gatts_if = gatts_if;
    	    is_connected = true;
    	    memcpy(&spp_remote_bda,&p_data->connect.remote_bda,sizeof(esp_bd_addr_t));
        	break;
    	case ESP_GATTS_DISCONNECT_EVT:
    	    is_connected = false;
    	    enable_data_ntf = false;
    	    esp_ble_gap_start_advertising(&spp_adv_params);
    	    break;
    	case ESP_GATTS_OPEN_EVT:
    	    break;
    	case ESP_GATTS_CANCEL_OPEN_EVT:
    	    break;
    	case ESP_GATTS_CLOSE_EVT:
    	    break;
    	case ESP_GATTS_LISTEN_EVT:
    	    break;
    	case ESP_GATTS_CONGEST_EVT:
    	    break;
    	case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
    	    ESP_LOGI( GATTS_TABLE_TAG, "The number handle =%x\n",
                    param->add_attr_tab.num_handle );
    	    if ( param->add_attr_tab.status != ESP_GATT_OK ) {
    	        ESP_LOGE(GATTS_TABLE_TAG, 
                        "Create attribute table failed, error code=0x%x",
                        param->add_attr_tab.status);
    	    }
    	    else if (param->add_attr_tab.num_handle != SPP_IDX_NB){
                ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table abnormally, "
                        "num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
                        param->add_attr_tab.num_handle, SPP_IDX_NB);
    	    }
    	    else {
    	        memcpy(spp_handle_table, param->add_attr_tab.handles,
                        sizeof(spp_handle_table));
    	        esp_ble_gatts_start_service(spp_handle_table[SPP_IDX_SVC]);
    	    }
    	    break;
    	}
    	default:
    	    break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, 
        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {

    ESP_LOGI(GATTS_TABLE_TAG, "EVT %d, gatts if %d\n", event, gatts_if);

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            spp_profile_tab[SPP_PROFILE_APP_IDX].gatts_if = gatts_if;
        } 
        else {
            ESP_LOGI(GATTS_TABLE_TAG, 
                    "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id, param->reg.status);
            return;
        }
    }

    /* Call the gatts_cb for the speicified gatts_if */
    int idx;
    /* Iterate through the profiles */
    for (idx = 0; idx < SPP_PROFILE_NUM; idx++) {
        /* ESP_GATT_IF_NONE, not specify a certain gatt_if,
         * need to call every profile cb function */
        if (gatts_if == ESP_GATT_IF_NONE || 
                gatts_if == spp_profile_tab[idx].gatts_if) {
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

    ret = esp_ble_gatts_app_register(ESP_SPP_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "%s gatts app register failed, error code = %x\n",
                __func__, ret);
    }
    else {
        ESP_LOGI(TAG, "[bt] Registered GATTS App");
    }

    cmd_cmd_queue = xQueueCreate(10, sizeof(char *));
    xTaskCreate(spp_cmd_task, "spp_cmd_task", 4096, NULL, 10, NULL);

    ESP_LOGI(TAG, "Done setting up bluetooth.");

}
