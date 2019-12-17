/* What works:
 *    *streams: ble_stdin, ble_stdout, ble_stderr
 * What doesn't work:
 *    *select
 *
 * For nimble error codes, see:
 *     https://mynewt.apache.org/latest/network/ble_hs/ble_hs_return_codes.html
 *     0x00XX - Core (nimble)
 *     0x01XX - ATT
 *     0x02XX - HCI
 *     0x03XX - L2CAP
 *     0x04XX - Security Manager (us)
 *     0x05XX - Security Manager (peer)
 */

#define LOG_LOCAL_LEVEL 4

#include "sdkconfig.h"

#if CONFIG_BT_ENABLED

    #include <stdarg.h>
    #include <sys/errno.h>
    #include <sys/fcntl.h>
    #include <sys/lock.h>
    #include <sys/param.h>
    #include "bluetooth.h"
    #include "driver/uart.h"
    #include "esp_console.h"
    #include "esp_log.h"
    #include "esp_system.h"
    #include "esp_timer.h"
    #include "esp_vfs_dev.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/event_groups.h"
    #include "freertos/portmacro.h"
    #include "freertos/task.h"
    #include "jolt_gui/jolt_gui.h"
    #include "jolt_helpers.h"
    #include "nvs_flash.h"
    #include "string.h"
    #include "jolt_gui/jolt_gui.h"

    /* BLE */
    #include "esp_nimble_hci.h"
    #include "nimble/nimble_port.h"
    #include "nimble/nimble_port_freertos.h"
    #include "host/ble_hs.h"
    #include "host/util/util.h"
    #include "console/console.h"
    #include "services/gap/ble_svc_gap.h"
    #include "services/gatt/ble_svc_gatt.h"

    #define GATTS_SEND_REQUIRE_CONFIRM false

    #if CONFIG_JOLT_BT_PROFILING
uint64_t ble_packet_cum_life = 0;
uint32_t ble_packet_n        = 0;
    #endif

FILE *ble_stdin;
FILE *ble_stdout;
FILE *ble_stderr;

static void bleprph_advertise(void);
static void bleprph_on_sync(void);

static int gap_event_handler(struct ble_gap_event *event, void *arg);
static int gatt_svr_chr_access_spp_read(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);
static int gatt_svr_chr_access_spp_write(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg);

static const char TAG[] = "bluetooth.c";

xQueueHandle ble_in_queue = NULL;

    #if CONFIG_JOLT_BT_DEBUG_ALWAYS_ADV
/* always in pairing mode for debugging */
bool jolt_bluetooth_pair_mode = true;
    #else
bool jolt_bluetooth_pair_mode = false;
    #endif

static uint8_t own_addr_type;
uint16_t spp_mtu_size;
static uint16_t conn_handle = 0;
static uint16_t resp_att_handle = 0;

/***************************
 *  SPP PROFILE ATTRIBUTES *
 ***************************/

/* DRIVER STUFF */
static int ble_open( const char *path, int flags, int mode );
static int ble_fstat( int fd, struct stat *st );
static int ble_close( int fd );
static ssize_t ble_write( int fd, const void *data, size_t size );
static ssize_t ble_read( int fd, void *data, size_t size );
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
typedef void ( *tx_func_t )( int, int );
// read bytes function type
typedef int ( *rx_func_t )( int );

static _lock_t s_ble_read_lock;
static _lock_t s_ble_write_lock;

/* Lock ensuring that uart_select is used from only one task at the time */

static int ble_open( const char *path, int flags, int mode )
{
    int fd = -1;
    if( strcmp( path, "/0" ) == 0 ) { fd = 0; }
    else {
        errno = ENOENT;
        return fd;
    }
    return fd;
}

static ssize_t ble_write( int fd, const void *data, size_t size )
{
    int rc;
    size_t remaining   = size;
    const char *data_c = (const char *)data;

    ESP_LOGD( TAG, "%s write %d bytes\n", __func__, size );

    _lock_acquire_recursive( &s_ble_write_lock );

    int idx = 0;
    do {
        esp_err_t res;
        size_t print_len = remaining;
        if( print_len > ( spp_mtu_size - 3 ) ) { print_len = ( spp_mtu_size - 3 ); }

        ESP_LOGD( TAG, "Sending %d bytes: %.*s", print_len, print_len, &data_c[idx] );

        struct os_mbuf* om;
        om = ble_hs_mbuf_from_flat(&data_c[idx], print_len);
        if( NULL == om ) { 
            abort();
        }
        rc =  ble_gattc_notify_custom(conn_handle, resp_att_handle, om);
        if( 0 != rc ) {
            abort();
        }

        idx += print_len;
        remaining -= print_len;
    } while( remaining > 0 );

    _lock_release_recursive( &s_ble_write_lock );
    return size;
}

    #define LINE_OFF_INVALID -1
static int peek_c = NONE;
int ble_read_char( int fd, TickType_t timeout )
{
    static int32_t line_off    = LINE_OFF_INVALID;  // offset into most recent queue item
    static ble_packet_t packet = {0};

    if( peek_c != NONE ) {
        char tmp = (char)peek_c;
        peek_c   = NONE;
        return tmp;
    }
    char c;

    if( line_off < 0 ) {
        if( !xQueueReceive( ble_in_queue, &packet, timeout ) ) return NONE;
        line_off = 0;
    }
    c = packet.data[line_off];
    line_off++;

    ESP_LOGD( TAG, "line_off: %d", line_off );
    ESP_LOGD( TAG, "packet.len: %d\n", packet.len );

    if( line_off >= packet.len ) {
        line_off = LINE_OFF_INVALID;
    #if CONFIG_JOLT_BT_PROFILING
        ble_packet_cum_life += esp_timer_get_time() - packet.t_receive;
        ble_packet_n++;
    #endif
        free( packet.data );
    }

    return c;
}

/* When we peek ahead to handle \r\n */
static void ble_return_char( int fd, int c ) { peek_c = c; }

static ssize_t ble_read( int fd, void *data, size_t size )
{
    ESP_LOGD( TAG, "%s: fd: %d, data: %p, size: %d", __func__, fd, data, size );
    return ble_read_timeout( fd, data, size, portMAX_DELAY );
}

ssize_t ble_read_timeout( int fd, void *data, size_t size, TickType_t timeout )
{
    char *data_c = (char *)data;

    ESP_LOGD( TAG, "ble_read_lock acquired" );
    ESP_LOGD( TAG, "Attempting to receive %d bytes with timeout %d.", size, timeout );

    _lock_acquire_recursive( &s_ble_read_lock );

    size_t received;
    for( received = 0; received < size; received++ ) {
        int c = ble_read_char( fd, timeout );

        if( c == NONE ) break;

        ESP_LOGD( TAG, "Char: %02X; Off: %d", (char)c, received );
        if( '\r' == (char)c ) {
            switch( s_rx_mode ) {
                case ESP_LINE_ENDINGS_CR: c = '\n'; break;
                case ESP_LINE_ENDINGS_CRLF: {
                    /* look ahead */
                    int c2 = ble_read_char( fd, portMAX_DELAY );
                    if( c2 == NONE ) {
                        /* could not look ahead, put the current character back */
                        ble_return_char( fd, (char)c );
                        break;
                    }
                    else if( c2 == '\n' ) {
                        /* this was \r\n sequence. discard \r, return \n */
                        c = '\n';
                    }
                    else {
                        /* \r followed by something else. put the second char back,
                         * it will be processed on next iteration. return \r now.
                         */
                        ble_return_char( fd, (char)c2 );
                    }
                    break;
                }
                default:
                    /* Do Nothing */
                    break;
            }
        }

        data_c[received] = (char)c;
    }

    #if LOG_LOCAL_LEVEL >= 4 /* debug */
    BLE_UART_LOG_STR( "ble_read_lock releasing\n" );
    #endif

    _lock_release_recursive( &s_ble_read_lock );

    if( received > 0 ) {
        /* Display what was received */
        BLE_UART_LOGD( "ble_read returning %d bytes\nreceived message: ", received );
        BLE_UART_LOGD_BUF( data_c, received - 1 );
        BLE_UART_LOGD_STR( "\n" );
    }

    if( received > 0 ) { return received; }
    errno = EWOULDBLOCK;
    return -1;
}

static int ble_fstat( int fd, struct stat *st )
{
    assert( fd == 0 );
    st->st_mode = S_IFCHR;
    return 0;
}

static int ble_close( int fd )
{
    /* Dummy since you don't really close this */
    assert( fd == 0 );
    return 0;
}

    #define CONFIG_JOLT_BLE_SPP_SELECT_EN 0
    #if CONFIG_JOLT_BLE_SPP_SELECT_EN
/* Stuff for select() */
static SemaphoreHandle_t *_signal_sem = NULL;
static fd_set *_readfds               = NULL;
static fd_set *_writefds              = NULL;
static fd_set *_errorfds              = NULL;
static fd_set *_readfds_orig          = NULL;
static fd_set *_writefds_orig         = NULL;
static fd_set *_errorfds_orig         = NULL;

static _lock_t s_one_select_lock;

static portMUX_TYPE ble_selectlock = portMUX_INITIALIZER_UNLOCKED;
static void ble_end_select();

static portMUX_TYPE *ble_get_selectlock() { return &ble_selectlock; }

/* SELECT notes
 *
 */
static void select_notif_callback( uart_port_t uart_num, uart_select_notif_t uart_select_notif,
                                   BaseType_t *task_woken )
{
    switch( uart_select_notif ) {
        case UART_SELECT_READ_NOTIF:
            if( FD_ISSET( uart_num, _readfds_orig ) ) {
                FD_SET( uart_num, _readfds );
                esp_vfs_select_triggered_isr( _signal_sem, task_woken );
            }
            break;
        case UART_SELECT_WRITE_NOTIF:
            if( FD_ISSET( uart_num, _writefds_orig ) ) {
                FD_SET( uart_num, _writefds );
                esp_vfs_select_triggered_isr( _signal_sem, task_woken );
            }
            break;
        case UART_SELECT_ERROR_NOTIF:
            if( FD_ISSET( uart_num, _errorfds_orig ) ) {
                FD_SET( uart_num, _errorfds );
                esp_vfs_select_triggered_isr( _signal_sem, task_woken );
            }
            break;
    }
}

static esp_err_t ble_start_select( int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                                   SemaphoreHandle_t *signal_sem )
{
    /* Setting up the environment for detection of read/write/error conditions
     * on file descriptors belonging to BLE VFS.
     */

    if( _lock_try_acquire( &s_one_select_lock ) ) { return ESP_ERR_INVALID_STATE; }
    portENTER_CRITICAL( ble_get_selectlock() );

        #define FD_ERR_CHECK( x, y )                       \
            if( x ) {                                      \
                portEXIT_CRITICAL( ble_get_selectlock() ); \
                ble_end_select();                          \
                return y;                                  \
            }
        #define FD_MEM_ALLOC( x, y ) FD_ERR_CHECK( NULL == ( x = malloc( sizeof( fd_set ) ) ), ESP_ERR_NO_MEM );

    FD_ERR_CHECK(
            _readfds || _writefds || _errorfds || _readfds_orig || _writefds_orig || _errorfds_orig || _signal_sem,
            ESP_ERR_INVALID_STATE );

    FD_MEM_ALLOC( _readfds_orig );
    FD_MEM_ALLOC( _writeds_orig );
    FD_MEM_ALLOC( _errorfds_orig );

        #undef FD_ERR_CHECK
        #undef FD_MEM_ALLOC

    /* Set Callbacks for all file descriptors*/
    for( int i = 0; i < nfds; ++i ) {
        if( FD_ISSET( i, readfds ) || FD_ISSET( i, writefds ) || FD_ISSET( i, exceptfds ) ) {
            ble_set_select_notif_callback( i, select_notif_callback );
        }
    }

    _signal_sem = signal_sem;

    _readfds  = readfds;
    _writefds = writefds;
    _errorfds = exceptfds;

    *_readfds_orig  = *readfds;
    *_writefds_orig = *writefds;
    *_errorfds_orig = *exceptfds;

    FD_ZERO( readfds );
    FD_ZERO( writefds );
    FD_ZERO( exceptfds );

    // todo; more stuff here

    return ESP_OK;
}

static void ble_end_select()
{
    portENTER_CRITICAL( ble_get_selectlock() );
    for( int i = 0; i < ble_NUM; ++i ) { ble_set_select_notif_callback( i, NULL ); }

    _signal_sem = NULL;

    _readfds  = NULL;
    _writefds = NULL;
    _errorfds = NULL;

    if( _readfds_orig ) {
        free( _readfds_orig );
        _readfds_orig = NULL;
    }

    if( _writefds_orig ) {
        free( _writefds_orig );
        _writefds_orig = NULL;
    }

    if( _errorfds_orig ) {
        free( _errorfds_orig );
        _errorfds_orig = NULL;
    }
    portEXIT_CRITICAL( ble_get_selectlock() );
    _lock_release( &s_one_select_lock );
    portENTER_CRITICAL( ble_get_selectlock() );
}
    #endif

void esp_vfs_dev_ble_spp_register()
{
    if( NULL == ble_in_queue ) { ble_in_queue = xQueueCreate( 50, sizeof( ble_packet_t ) ); }

    esp_vfs_t vfs = {
        .flags = ESP_VFS_FLAG_DEFAULT,
        .write = &ble_write,
        .open  = &ble_open,
        .fstat = &ble_fstat,
        .close = &ble_close,
        .read  = &ble_read,
    #if CONFIG_JOLT_BLE_SPP_SELECT_EN
        .start_select = &ble_start_select,
        .end_select   = &ble_end_select,
    #endif
    };
    ESP_ERROR_CHECK( esp_vfs_register( "/dev/ble", &vfs, NULL ) );
}

void ble_set_rx_line_endings( esp_line_endings_t mode ) { s_rx_mode = mode; }

/* END DRIVER STUFF */

/**
 * Logs information about a connection to the console.
 */
static void
bleprph_print_conn_desc(struct ble_gap_conn_desc *desc)
{
    uint8_t *u8p;

    u8p = desc->our_ota_addr.val;
    ESP_LOGI(TAG, "handle=%d our_ota_addr_type=%d our_ota_addr="
            "%02x:%02x:%02x:%02x:%02x:%02x",
                desc->conn_handle, desc->our_ota_addr.type,
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);


    u8p = desc->our_id_addr.val;
    ESP_LOGI(TAG, " our_id_addr_type=%d our_id_addr="
            "%02x:%02x:%02x:%02x:%02x:%02x",
                desc->our_id_addr.type,
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    u8p = desc->peer_ota_addr.val;
    ESP_LOGI(TAG, " peer_ota_addr_type=%d peer_ota_addr="
            "%02x:%02x:%02x:%02x:%02x:%02x",
                desc->peer_ota_addr.type,
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    u8p = desc->peer_id_addr.val;
    ESP_LOGI(TAG, " peer_id_addr_type=%d peer_id_addr="
            "%02x:%02x:%02x:%02x:%02x:%02x",
                desc->peer_id_addr.type,
                u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);

    ESP_LOGI(TAG, " conn_itvl=%d conn_latency=%d supervision_timeout=%d "
                "encrypted=%d authenticated=%d bonded=%d\n",
                desc->conn_itvl, desc->conn_latency,
                desc->supervision_timeout,
                desc->sec_state.encrypted,
                desc->sec_state.authenticated,
                desc->sec_state.bonded);
}

static bool is_connected = false;

bool jolt_bluetooth_is_connected() { return is_connected; }

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleuart uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unuesd by
 *                                  bleuart.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int gap_event_handler(struct ble_gap_event *event, void *arg)
    {
    struct ble_gap_conn_desc desc;
    int rc;

    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            /* A new connection was established or a connection attempt failed. */
            // TODO: if not authenticated, call ble_gap_security_initiate() ?
            ESP_LOGI(TAG, "connection %s; status=%d ",
                        event->connect.status == 0 ? "established" : "failed",
                        event->connect.status);
            if (event->connect.status == 0) {
                rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
                assert(rc == 0);
                bleprph_print_conn_desc(&desc);
                is_connected = true;
            }

            if (event->connect.status == 0) {
                // TODO see what to really do here
                conn_handle = event->connect.conn_handle;
                //ble_gap_security_initiate(event->connect.conn_handle);
            }

            if (event->connect.status != 0) {
                /* Connection failed; resume advertising. */
                bleprph_advertise();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            ESP_LOGI(TAG, "disconnect; reason=0x%04X", event->disconnect.reason);
            bleprph_print_conn_desc(&event->disconnect.conn);
            is_connected = false;

            /* Connection terminated; resume advertising. */
            conn_handle = 0;
            bleprph_advertise();
            break;

        case BLE_GAP_EVENT_CONN_UPDATE:
            /* The central has updated the connection parameters. */
            ESP_LOGI(TAG, "connection updated; status=%d ",
                        event->conn_update.status);
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
            break;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            ESP_LOGI(TAG, "advertise complete; reason=0x%04X",
                        event->adv_complete.reason);
            bleprph_advertise(); // TODO: I think this will continuously advertise.
            break;

        case BLE_GAP_EVENT_ENC_CHANGE:
            /* Encryption has been enabled or disabled for this connection. */
            ESP_LOGI(TAG, "encryption change event; status=%d ",
                        event->enc_change.status);
            rc = ble_gap_conn_find(event->connect.conn_handle, &desc);
            assert(rc == 0);
            bleprph_print_conn_desc(&desc);
            break;

        case BLE_GAP_EVENT_SUBSCRIBE:
            ESP_LOGI(TAG, "subscribe event; conn_handle=%d attr_handle=%d "
                        "reason=0x%04X prevn=%d curn=%d previ=%d curi=%d\n",
                        event->subscribe.conn_handle,
                        event->subscribe.attr_handle,
                        event->subscribe.reason,
                        event->subscribe.prev_notify,
                        event->subscribe.cur_notify,
                        event->subscribe.prev_indicate,
                        event->subscribe.cur_indicate);
            break;

        case BLE_GAP_EVENT_MTU:
            ESP_LOGI(TAG, "mtu update event; conn_handle=%d cid=%d mtu=%d\n",
                        event->mtu.conn_handle,
                        event->mtu.channel_id,
                        event->mtu.value);
            spp_mtu_size = event->mtu.value;
            break;

        case BLE_GAP_EVENT_REPEAT_PAIRING:
            /* We already have a bond with the peer, but it is attempting to
             * establish a new secure link.  This app sacrifices security for
             * convenience: just throw away the old bond and accept the new link.
             */

            /* Delete the old bond. */
            rc = ble_gap_conn_find(event->repeat_pairing.conn_handle, &desc);
            assert(rc == 0);
            ble_store_util_delete_peer(&desc.peer_id_addr);

            /* Return BLE_GAP_REPEAT_PAIRING_RETRY to indicate that the host should
             * continue with the pairing operation.
             */
            return BLE_GAP_REPEAT_PAIRING_RETRY;

        case BLE_GAP_EVENT_PASSKEY_ACTION:
            // handled in bluetooth gui callback
            ESP_LOGI(TAG, "PASSKEY_ACTION_EVENT started \n");
            break;
    }

    return jolt_gui_gap_cb( event, arg );
}

static int gatt_svr_chr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                   void *dst, uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    /**
     * l
     *
     * TODO
     *     * Flags: AUTHEN vs AUTHOR
     *
     */
    /** Service: SPP */
	{
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(0xFFE0), 
        .characteristics    = (struct ble_gatt_chr_def[]) {
            {
                /* Read (Jolt -> smartphone/computer) */
                .uuid = BLE_UUID16_DECLARE(ESP_GATT_UUID_SPP_DATA_NOTIFY),
                .access_cb = gatt_svr_chr_access_spp_read,
                .flags = BLE_GATT_CHR_F_NOTIFY,
                .val_handle = &resp_att_handle, 
            }, {
                /* Write (smartphone/computer -> Jolt) */
                .uuid = BLE_UUID16_DECLARE(ESP_GATT_UUID_SPP_COMMAND_RECEIVE),
                .access_cb = gatt_svr_chr_access_spp_write,
                // TODO
                //.flags = BLE_GATT_CHR_F_WRITE_ENC | BLE_GATT_CHR_F_WRITE_AUTHEN | BLE_GATT_CHR_F_WRITE_NO_RSP,
                .flags = BLE_GATT_CHR_F_WRITE, // works!
                //.flags = BLE_GATT_CHR_F_WRITE  | BLE_GATT_CHR_F_WRITE_ENC,
            }, {
                0, /* No more characteristics in this service. */
            }
        }
	},
    {
        0, /* No more services. */
    },
};

static int gatt_svr_chr_access_spp_read(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg)
{
    ESP_LOGD(TAG, "(%d) READ", __LINE__);
    return 0;
}

/**
 * @brief Write (smartphone/computer -> Jolt)
 */
static int gatt_svr_chr_access_spp_write(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt,
                             void *arg)
{
    ESP_LOGD(TAG, "(%d) WRITE", __LINE__);
    int rc = 0;
    assert(ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR);

    {
#if 1
        /* Forward packet to the ble_in_queue */
        ble_packet_t packet = {0};
#if CONFIG_JOLT_BT_PROFILING
        packet.t_receive = esp_timer_get_time();
#endif
        ESP_LOGI( TAG, " Allocating %d bytes.", OS_MBUF_PKTLEN(ctxt->om) );
        packet.data = (uint8_t *)malloc( OS_MBUF_PKTLEN(ctxt->om) );
        if( packet.data == NULL ) {
            ESP_LOGE( TAG, "%s malloc failed\n", __func__ );
            rc = 1; // TODO better rc
            goto exit;
        }
        packet.len = OS_MBUF_PKTLEN(ctxt->om);
        uint16_t written_len;
        rc = ble_hs_mbuf_to_flat(ctxt->om, packet.data, packet.len, &written_len);
        if( !xQueueSend( ble_in_queue, &packet, 10 / portTICK_PERIOD_MS ) ) {
            ESP_LOGE( TAG, "Timed out trying to put packet onto ble_in_queue" );
            free( packet.data );
            rc = 1; // TODO better rc
            goto exit;
        }

#endif
    }

exit:
    return rc;
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op) {
    case BLE_GATT_REGISTER_OP_SVC:
        ESP_LOGD(TAG, "registered service %s with handle=%d",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        ESP_LOGD(TAG, "    registering characteristic %s with "
                    "def_handle=%d val_handle=%d",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        ESP_LOGD(TAG, "    registering descriptor %s with handle=%d",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

static void bleprph_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    nimble_port_freertos_deinit();
}

static void bleprph_on_reset(int reason)
{
    ESP_LOGE(TAG, "Resetting state; reason=0x%04X\n", reason);
}

esp_err_t jolt_bluetooth_start()
{
    esp_err_t err = ESP_OK;

    /* BT Controller and initialization */
    {
        err = esp_nimble_hci_and_controller_init();
        switch( err ) {
            case ESP_OK:
                break;
            default:
                ESP_LOGE(TAG, "Error initializing BLE HCI and Controller (%s).", esp_err_to_name(err));
                abort();
                goto exit;
                break;
        }
    }

    nimble_port_init();

    {
        /* config */

		/** @brief Stack sync callback
		 *
		 * This callback is executed when the host and controller become synced.
		 * This happens at startup and after a reset.
		 */
		ble_hs_cfg.sync_cb = bleprph_on_sync;
		ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
		ble_hs_cfg.gatts_register_cb = gatt_svr_register_cb;
        ble_hs_cfg.reset_cb = bleprph_on_reset;

        ble_hs_cfg.sm_io_cap = BLE_SM_IO_CAP_DISP_ONLY;
        ble_hs_cfg.sm_bonding = 1;  // TODO: uncomment
        ble_hs_cfg.sm_mitm  = 1;
        ble_hs_cfg.sm_sc  = 1;
        ble_hs_cfg.sm_our_key_dist = 1;
        ble_hs_cfg.sm_their_key_dist = 1;
    }

    /* Set the default device name. */
    {
        int rc;
        rc = ble_svc_gap_device_name_set(BLE_DEFAULT_DEVICE_NAME);
        assert(rc == 0);
    }

    ble_svc_gap_init();
    ble_svc_gatt_init();

	{
		int rc;
		rc = ble_gatts_count_cfg(gatt_svr_svcs); // TODO
		if (rc != 0) {
            goto exit;
		}

		rc = ble_gatts_add_svcs(gatt_svr_svcs); // TODO
		if (rc != 0) {
            goto exit;
		}
	}

    nimble_port_freertos_init(bleprph_host_task);

exit:
    return ESP_OK;
}

esp_err_t jolt_bluetooth_stop()
{
    // TODO
    
#if 0
    esp_err_t err = ESP_OK;

    /* Destroy Bluedroid */
    {
        esp_bluedroid_status_t status;
        status = esp_bluedroid_get_status();
        switch( status ) {
            /* Falls through */
            case ESP_BLUEDROID_STATUS_ENABLED:
                err = esp_bluedroid_disable();
                if( ESP_OK != err ) {
                    ESP_LOGE( TAG,
                              "%s bluedroid disable failed, "
                              "error code = %s\n",
                              __func__, esp_err_to_name( err ) );
                    goto exit;
                }
            /* Falls through */
            case ESP_BLUEDROID_STATUS_INITIALIZED:
                err = esp_bluedroid_deinit();
                if( ESP_OK != err ) {
                    ESP_LOGE( TAG,
                              "%s bluedroid deinit failed, "
                              "error code = %s\n",
                              __func__, esp_err_to_name( err ) );
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
        switch( status ) {
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_ENABLED:
                err = esp_bt_controller_disable();
                if( ESP_OK != err ) {
                    ESP_LOGE( TAG,
                              "%s bt controller disable failed, "
                              "error code = %s\n",
                              __func__, esp_err_to_name( err ) );
                    goto exit;
                }
            /* Falls through */
            case ESP_BT_CONTROLLER_STATUS_INITED:
                err = esp_bt_controller_deinit();
                if( ESP_OK != err ) {
                    ESP_LOGE( TAG,
                              "%s bt controller deinit failed, "
                              "error code = %s\n",
                              __func__, esp_err_to_name( err ) );
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
#else
    return ESP_OK;
#endif
}

/**
 * Enables advertising with the following parameters:
 *     o General discoverable mode.
 *     o Undirected connectable mode.
 */
static void bleprph_advertise(void)
{
    ESP_LOGI(TAG, "BLE Advertising start");

    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields = { 0 };
    const char *name;
    int rc;

    /**
     *  Set the advertisement data included in our advertisements:
     *     o Flags (indicates advertisement type and other general info).
     *     o Advertising tx power.
     *     o Device name.
     *     o 16-bit service UUIDs (alert notifications).
     */

    /* Advertise two flags:
     *     o Discoverability in forthcoming advertisement (general)
     *     o BLE-only (BR/EDR unsupported).
     */
    fields.flags = BLE_HS_ADV_F_DISC_GEN |
                   BLE_HS_ADV_F_BREDR_UNSUP;

    /* Indicate that the TX power level field should be included; have the
     * stack fill this value automatically.  This is done by assigning the
     * special value BLE_HS_ADV_TX_PWR_LVL_AUTO.
     */
    fields.tx_pwr_lvl_is_present = 1;
    fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

    name = ble_svc_gap_device_name();
    fields.name = (uint8_t *)name;
    fields.name_len = strlen(name);
    fields.name_is_complete = 1;

    fields.uuids16 = (ble_uuid16_t[]) {
        BLE_UUID16_INIT(0xFFE0)
        //BLE_UUID16_INIT(GATT_SVR_SVC_ALERT_UUID)
    };
    fields.num_uuids16 = 1;
    fields.uuids16_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) {
        ESP_LOGE(TAG, "error setting advertisement data; rc=%d\n", rc);
        return;
    }

    /* Begin advertising. */
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    rc = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                           &adv_params, gap_event_handler, NULL);
    if (rc != 0) {
        ESP_LOGE(TAG, "error enabling advertisement; rc=%d\n", rc);
        return;
    }
}

static void bleprph_on_sync(void)
{
    int rc;

    rc = ble_hs_util_ensure_addr(0);
    assert(rc == 0);

    /* Figure out address to use while advertising (no privacy for now) */
    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0) {
        MODLOG_DFLT(ERROR, "error determining address type; rc=%d\n", rc);
        return;
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    rc = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);

    {
        uint8_t *u8p = addr_val;
        ESP_LOGI(TAG, "Device Address: %02x:%02x:%02x:%02x:%02x:%02x",
                    u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
    }

    /* Begin advertising. */
    bleprph_advertise();
}


/* Start Advertising to ALL devices; useful for pairing */
esp_err_t jolt_bluetooth_adv_all_start()
{
    // TODO
    bleprph_advertise();
    return ESP_OK;
}

/* Start Advertising to whitelisted devices */
esp_err_t jolt_bluetooth_adv_wht_start()
{
    // TODO
    bleprph_advertise();
    return ESP_OK;
}

/* Stop advertising */
esp_err_t jolt_bluetooth_adv_stop() {
    // TODO
    return ESP_OK;
}

void jolt_bluetooth_config_security( bool bond ) {
    // TODO
}

#else

/* Stubs */
esp_err_t jolt_bluetooth_start() { return ESP_OK; }

esp_err_t jolt_bluetooth_stop() { return ESP_OK; }

#endif
