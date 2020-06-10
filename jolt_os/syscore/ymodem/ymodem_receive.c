/*
 * ESP32 YModem driver
 *
 * Copyright (C) LoBo 2017
 *
 * Author: Boris Lovosevic (loboris@gmail.com)
 *
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.    In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

//#define LOG_LOCAL_LEVEL 3

#include <driver/uart.h>
#include <esp_timer.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/select.h>
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "jolt_helpers.h"
#include "syscore/cli.h"
#include "syscore/decompress.h"
#include "syscore/filesystem.h"
#include "syscore/ymodem.h"
#include "syscore/ymodem/ymodem_common.h"

#if CONFIG_JOLT_BT_YMODEM_PROFILING
uint64_t t_ymodem_send = 0, t_ymodem_receive = 0;

static uint64_t t_ymodem_first_byte_cum = 0;
static uint32_t n_ymodem_first_byte     = 0;
static uint64_t t_ymodem_packet_cum     = 0;
static uint32_t n_ymodem_packet         = 0;
#endif

#define RECEIVE_PACKET_OK 0

/**
 * @brief    Receive a packet from sender
 * @param[out]    data
 * @param[out]    length
 *        >0: packet length
 *         0: end of transmission
 *        -1: abort by sender
 *        -2: error or crc error
 * @param[in]    timeout
 * @retval 0: normal return (successful communication according to protocol)
 */
static int32_t receive_packet( uint8_t *data, int *length, uint32_t timeout )
{
    int packet_size, rec_bytes;
    unsigned char ch;
    *length = 0;

    // receive 1st byte
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    uint64_t t_start = esp_timer_get_time();
#endif
    if( receive_byte( &ch, timeout ) < 0 ) { return YMODEM_ERR_TIMEOUT; }
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_first_byte_cum += esp_timer_get_time() - t_start;
    n_ymodem_first_byte++;
#endif

    switch( ch ) {
        case SOH: packet_size = PACKET_SIZE; break;
        case STX: packet_size = PACKET_1K_SIZE; break;
        case EOT: *length = 0; return RECEIVE_PACKET_OK;
        case CA: /* An actual cancellation requires two cancels back to back */
            if( receive_byte( &ch, timeout ) < 0 ) { return YMODEM_ERR_TIMEOUT; }
            if( ch == CA ) {
                // Abort by sender
                *length = YMODEM_ERR_ABORT_BY_USER;
                return RECEIVE_PACKET_OK;
            }
            else
                return YMODEM_ERR_PROTOCOL;
        case ABORT1:
        case ABORT2: return YMODEM_ERR_ABORT_BY_USER;
        default:
            vTaskDelay( 100 / portTICK_RATE_MS );
            rx_consume();
            return YMODEM_ERR_TIMEOUT;
    }
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_packet_cum -= esp_timer_get_time();
#endif

    {
        *data         = (uint8_t)ch;
        uint8_t *dptr = data + 1;
        rec_bytes     = receive_bytes( dptr, timeout, packet_size + PACKET_OVERHEAD - 1 );
    }

#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_packet_cum += esp_timer_get_time();
    n_ymodem_packet++;
#endif

    if( rec_bytes < 0 ) {
        BLE_UART_LOGI( "%d) timeout receive_bytes", __LINE__ );
        return ABORT_BY_TIMEOUT;
    }

    /* Check complimentary sequence number */
    if( data[PACKET_SEQNO_INDEX] != ( ( data[PACKET_SEQNO_COMP_INDEX] ^ 0xff ) & 0xff ) ) {
        *length = -2;
        return RECEIVE_PACKET_OK;
    }
    if( crc16( &data[PACKET_HEADER], packet_size + PACKET_TRAILER ) != 0 ) {
        *length = -3;
        return RECEIVE_PACKET_OK;
    }

    *length = packet_size;
    return RECEIVE_PACKET_OK;
}

// Receive a file using the ymodem protocol.
//-----------------------------------------------------------------
#define SEND_CA_EXIT( x ) \
    send_CA();            \
    size = x;             \
    goto exit;
#define SEND_ACK_EXIT( x ) \
    send_ACK();            \
    size = x;              \
    goto exit;
int ymodem_receive_write( void *ffd, unsigned int maxsize, char *getname, write_fun_t write_fun,
                                    int8_t *progress )
{
    /* Just incase stdin is from uart, we override the current settings to
     * potentially replace carriage returns */
    esp_vfs_dev_uart_set_rx_line_endings( -1 );
    ble_set_rx_line_endings( -1 );
    jolt_suspend_logging();

    char name[JOLT_FS_MAX_FILENAME_BUF_LEN] = {0};
    uint8_t *packet_data;
    int size = 0;
    uint8_t *file_ptr;
    char file_size[FILE_SIZE_LENGTH + 1];
    unsigned int i, file_len, write_len, session_done, file_done, packets_received, errors = 0;
    int packet_length = 0;
    file_len          = 0;
    int eof_cnt       = 0;

#if CONFIG_JOLT_COMPRESSION_AUTO
    static decomp_t *d = NULL;
#endif

    vTaskPrioritySet( NULL, 17 );

    /* Profiling Variables */
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    uint64_t t_ymodem_start = 0, t_ymodem_end = 0;
    uint64_t t_disk  = 0;
    t_ymodem_send    = 0;
    t_ymodem_receive = 0;
    t_ymodem_start   = esp_timer_get_time();
#endif

    packet_data = malloc( PACKET_1K_SIZE + PACKET_OVERHEAD );
    if( NULL == packet_data ) { goto exit; }

    jolt_cli_suspend();

    send_CRC16(); /* Initiate Transfer */

    for( session_done = 0, errors = 0;; ) {
        for( packets_received = 0, file_done = 0;; ) {
            if( size > 0 && NULL != progress ) {
                /* update progress value */
                *progress = file_len * 100 / size;
            }
            switch( receive_packet( packet_data, &packet_length, NAK_TIMEOUT ) ) {
                case RECEIVE_PACKET_OK:
                    if(packet_length < 0) {
                        switch( packet_length ) {
                            case YMODEM_ERR_ABORT_BY_USER:
                                // Abort by sender
                                SEND_ACK_EXIT( YMODEM_ERR_ABORT_BY_USER );
                            case -2:
                                BLE_UART_LOGW( "%d) Case -2", __LINE__ );
                                /* Fall Through */
                            case -3:
                                // error
                                BLE_UART_LOGW( "%d) Case -3", __LINE__ );

                                errors++;
                                if( errors > 5 ) { SEND_CA_EXIT( -2 ); }
                                send_NAK();
                                break;
                            default:
                                assert( 0 );
                        break;
                    }
                    if(packet_length == 0) {
                        // End of transmission
                        eof_cnt++;
                        BLE_UART_LOGI( "%d) EOT %d", __LINE__, eof_cnt );
                        if( eof_cnt == 1 ) { send_NAK(); }
                        else {
                            send_ACKCRC16();
                        }
                        break;
                    }
                    // ** Normal packet **
                    if( eof_cnt > 1 ) { send_ACK(); }
                    if( ( packet_data[PACKET_SEQNO_INDEX] & 0xff ) !=
                             ( packets_received & 0x000000ff ) ) {
                        BLE_UART_LOGE( "%d) Incorrect PACKET_SEQNO %02x; expected %02x", __LINE__,
                                       packet_data[PACKET_SEQNO_INDEX] & 0xff, packets_received & 0x000000ff );
                        errors++;
                        if( errors > MAX_ERRORS ) { SEND_CA_EXIT( -3 ); }
                        send_NAK();
                        break;
                    }
                    if( packets_received == 0 ) {
#if CONFIG_JOLT_BT_YMODEM_PROFILING
                        ymodem_transfer_in_progress = true;
#endif
                        // ** First packet, Filename packet **
                        if( packet_data[PACKET_HEADER] != 0 ) {
                            errors = 0;
                            // ** Filename packet has valid data
                            {
                                char *name_ptr;
                                for( name_ptr = name, i = 0, file_ptr = packet_data + PACKET_HEADER;
                                     *file_ptr != '\0'; i++ ) {
                                    if( i >= JOLT_FS_MAX_FILENAME_LEN ) {
                                        /* Filename too long */
                                        SEND_CA_EXIT( -13 );
                                    }
                                    *name_ptr++ = *file_ptr++;
                                }
                                *name_ptr = '\0';
                                if( NULL != getname ) {
                                    if( '\0' == getname[0] ) {
                                        /* Copy over string */
                                        strlcpy( getname, name, JOLT_FS_MAX_FILENAME_BUF_LEN );
                                    }
                                    /* Compare string */
                                    else if( 0 != strcmp( name, getname ) ) {
                                        SEND_CA_EXIT( -14 );
                                    }
                                }

#if CONFIG_JOLT_COMPRESSION_AUTO
                                /* Check if the suffix is ".gz" */
                                if( 0 == strcmp( name_ptr - 3, ".gz" ) ) {
                                    d = decompress_obj_init( write_fun, ffd );
                                    if( NULL == d ) { SEND_CA_EXIT( -12 ); }
                                    /* remove the ".gz" suffix */
                                    name_ptr -= 3;
                                    *name_ptr = '\0';
                                }
#endif
                            }
                            for( i = 0, file_ptr++; *file_ptr != ' ' && *file_ptr != '\0'; ) {
                                if( i >= FILE_SIZE_LENGTH ) {
                                    /* This file is definitely too big */
                                    size = -9;
                                    goto exit;
                                }
                                file_size[i++] = *file_ptr++;
                            }
                            file_size[i++] = '\0';
                            if( strlen( file_size ) > 0 )
                                size = strtol( file_size, NULL, 10 );
                            else
                                size = 0;
                            BLE_UART_LOGI( "Header indicates file is %d long.\n", size );

                            // Test the size of the file
                            if( ( size < 1 ) || ( size > maxsize ) ) {
                                // End session
                                send_CA();
                                if( size > maxsize ) { size = -9; }
                                else {
                                    size = -4;
                                }
                                goto exit;
                            }

                            file_len = 0;
                            send_ACKCRC16();
                        }
                        // Filename packet is empty, end session
                        else {
                            errors++;
                            if( errors > 5 ) { SEND_CA_EXIT( -5 ); }
                            send_NAK();
                        }
                    }
                    else {
                        // ** Data packet **
                        if( stdin == ble_stdin ) send_ACK();  // Send ACK early
                        BLE_UART_LOGD( "%d) Received Data Packet", __LINE__ );
                        // Write received data to file
                        if( file_len < size ) {
                            file_len += packet_length;  // total bytes received
                            if( file_len > size ) {
                                /* Truncate the final packet */
                                write_len = packet_length - ( file_len - size );
                                file_len  = size;
                            }
                            else {
                                write_len = packet_length;
                            }

                            int written_bytes;
#if CONFIG_JOLT_BT_YMODEM_PROFILING
                            uint64_t t_disk_start = esp_timer_get_time();
#endif
#if CONFIG_JOLT_COMPRESSION_AUTO
                            if( NULL != d ) {
                                decompress_obj_chunk( d, (uint8_t *)( packet_data + PACKET_HEADER ),
                                                      write_len );
                                written_bytes = write_len;  // tmp workaround
                            }
                            else
#endif
                            {
#if 1
                                BLE_UART_LOGD( "Writing %d bytes to disk.\n", write_len );
                                written_bytes = write_fun( (char *)( packet_data + PACKET_HEADER ), 1,
                                                           write_len, ffd );
                                BLE_UART_LOGD( "%d bytes written to disk.\n", file_len );
#else
                                written_bytes = write_len;
#endif
                            }
#if CONFIG_JOLT_BT_YMODEM_PROFILING
                            t_disk += esp_timer_get_time() - t_disk_start;
#endif

                            if( written_bytes != write_len ) {  // failed
                                /* End session */
                                SEND_CA_EXIT( -6 );
                            }
                        }
                        // success
                        errors = 0;
                        if( stdin != ble_stdin ) send_ACK();
                        taskYIELD();
                    }
                    packets_received++;
                    break;
                case YMODEM_ERR_ABORT_BY_USER:
                    SEND_CA_EXIT( YMODEM_ERR_ABORT_BY_USER );
                case YMODEM_ERR_PROTOCOL:
                    /* Falls through */
                case YMODEM_ERR_TIMEOUT:
                    BLE_UART_LOGD( "%d) TIMEOUT", __LINE__ );
                    if( eof_cnt > 1 ) { file_done = 1; }
                    else {
                        errors++;
                        if( errors > MAX_ERRORS ) { SEND_CA_EXIT( -8 ); }
                        else if( size == 0 ) {
                            send_CRC16();
                        }
                        else {
                            send_NAK();
                        }
                    }
                    break;
                default:
                    assert(0);  // Better to just hard crash
            }
            if( file_done != 0 ) {
                session_done = 1;
                break;
            }
        }
        if( session_done != 0 ) { break; }
    }

exit:

#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_end = esp_timer_get_time();
#endif
#if CONFIG_JOLT_COMPRESSION_AUTO
    decompress_obj_del( d );
#endif

    vTaskPrioritySet( NULL, CONFIG_JOLT_TASK_PRIORITY_BACKGROUND );

    jolt_cli_resume();

    jolt_resume_logging();
    if( NULL != packet_data ) { free( packet_data ); }
    BLE_UART_LOGI( "\n%d errors corrected during transfer.", errors );
    if( NULL != progress && size < 0 ) { *progress = -1; }
    esp_vfs_dev_uart_set_rx_line_endings(
#if CONFIG_NEWLIB_STDIN_LINE_ENDING_CRLF
            ESP_LINE_ENDINGS_CRLF
#elif CONFIG_NEWLIB_STDIN_LINE_ENDING_CR
            ESP_LINE_ENDINGS_CR
#else
            ESP_LINE_ENDINGS_LF
#endif
    );

    ble_set_rx_line_endings(
#if CONFIG_NEWLIB_STDIN_LINE_ENDING_CRLF
            ESP_LINE_ENDINGS_CRLF
#elif CONFIG_NEWLIB_STDIN_LINE_ENDING_CR
            ESP_LINE_ENDINGS_CR
#else
            ESP_LINE_ENDINGS_LF
#endif
    );

    if( size <= 0 ) BLE_UART_LOGE( "Error %d", size );

#if CONFIG_JOLT_BT_YMODEM_PROFILING
    BLE_UART_LOGI( "\n-----------------------------\n"
                   "YMODEM Profiling Results (%d Bytes):\n"
                   "Description            Time (uS)\n"
                   "Receives:              %8d\n"
                   "Sends:                 %8d\n"
                   "Disk Writing           %8d\n"
    #if CONFIG_JOLT_BT_PROFILING
                   "Avg BLE Packet Life    %8d\n"
    #endif
                   "Avg ymodem Packet Wait %8d\n"
                   "Avg first byte wait    %8d\n"
                   "t_ble_read_timeout     %8d\n"
                   "TOTAL                  %d\n"
                   "-----------------------------\n\n",
                   size, (uint32_t)t_ymodem_receive, (uint32_t)t_ymodem_send, (uint32_t)t_disk,
    #if CONFIG_JOLT_BT_PROFILING
                   ( uint32_t )( ble_packet_cum_life / ble_packet_n ),
    #endif
                   ( uint32_t )( t_ymodem_packet_cum / n_ymodem_packet ),
                   ( uint32_t )( t_ymodem_first_byte_cum / n_ymodem_first_byte ), (uint32_t)t_ble_read_timeout,
                   ( uint32_t )( t_ymodem_end - t_ymodem_start ) );
#endif

    return size;
}

// Receive a file using the ymodem protocol.
//-----------------------------------------------------------------
int ymodem_receive( FILE *ffd, unsigned int maxsize, char *getname, int8_t *progress )
{
    return ymodem_receive_write( ffd, maxsize, getname, (write_fun_t)&fwrite, progress );
}
