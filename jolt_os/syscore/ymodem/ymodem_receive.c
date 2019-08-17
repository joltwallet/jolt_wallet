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

#define LOG_LOCAL_LEVEL 3

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/select.h>
#include <sys/errno.h>
#include "esp_vfs.h"
#include "esp_vfs_dev.h"

#include "syscore/cli.h"
#include "syscore/ymodem.h"
#include "syscore/ymodem/ymodem_common.h"
#include <driver/uart.h>
#include "esp_log.h"
#include <esp_timer.h>
#include "syscore/decompress.h"
#include "syscore/filesystem.h"
#include "jolt_helpers.h"

#if CONFIG_JOLT_BT_YMODEM_PROFILING
uint64_t t_ymodem_send = 0, t_ymodem_receive = 0;

static uint64_t t_ymodem_first_byte_cum = 0;
static uint32_t n_ymodem_first_byte = 0;
static uint64_t t_ymodem_packet_cum = 0;
static uint32_t n_ymodem_packet = 0;
#endif

/**
    * @brief    Receive a packet from sender
    * @param[out]    data
    * @param[out]    length
    * @param[in]    timeout
    *        >0: packet length
    *         0: end of transmission
    *        -1: abort by sender
    *        -2: error or crc error
    * @retval 0: normally return
    *                -1: timeout
    *                -2: abort by user
    */
//--------------------------------------------------------------------------
static int32_t IRAM_ATTR receive_packet(uint8_t *data, int *length, uint32_t timeout)
{
    int count, packet_size, rec_bytes;
    unsigned char ch;
    *length = 0;
    
    // receive 1st byte
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    uint64_t t_start = esp_timer_get_time();
#endif
    if (receive_byte(&ch, timeout) < 0) {
        return -1;
    }
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_first_byte_cum += esp_timer_get_time() - t_start;
    n_ymodem_first_byte++;
#endif

    switch (ch) {
        case SOH:
            packet_size = PACKET_SIZE;
            break;
        case STX:
            packet_size = PACKET_1K_SIZE;
            break;
        case EOT:
            *length = 0;
            return 0;
        case CA:
            if (receive_byte(&ch, timeout) < 0) {
                return ABORT_BY_USER;
            }
            if (ch == CA) {
                *length = -1;
                return 0;
            }
            else return -1;
        case ABORT1:
        case ABORT2:
            return ABORT_BY_USER;
        default:
            vTaskDelay(100 / portTICK_RATE_MS);
            rx_consume();
            return ABORT_BY_TIMEOUT;
    }
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_packet_cum -= esp_timer_get_time();
#endif

    {
        *data = (uint8_t)ch;
        uint8_t *dptr = data+1;
        count = packet_size + PACKET_OVERHEAD-1;
        rec_bytes = receive_bytes(dptr, timeout, count);
    }

#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_packet_cum += esp_timer_get_time();
    n_ymodem_packet++;
#endif

    if( rec_bytes < 0 ) {
        BLE_UART_LOGI("%d) timeout receive_bytes", __LINE__);
        return ABORT_BY_TIMEOUT;
    }

    /* Check complimentary sequence number */
    if (data[PACKET_SEQNO_INDEX] != ((data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)) {
        *length = -2;
        return 0;
    }
    if (crc16(&data[PACKET_HEADER], packet_size + PACKET_TRAILER) != 0) {
        *length = -3;
        return 0;
    }


    *length = packet_size;
    return 0;
}

// Receive a file using the ymodem protocol.
//-----------------------------------------------------------------
#define SEND_CA_EXIT(x) send_CA(); size=x; goto exit;
#define SEND_ACK_EXIT(x) send_ACK(); size=x; goto exit;
int IRAM_ATTR ymodem_receive_write (void *ffd, unsigned int maxsize, char* getname,
                write_fun_t write_fun, int8_t *progress) {
    /* Just incase stdin is from uart, we override the current settings to 
     * potentially replace carriage returns */
    esp_vfs_dev_uart_set_rx_line_endings(-1);
    ble_set_rx_line_endings(-1);
    esp_log_level_set("*", ESP_LOG_NONE);

    uint8_t *packet_data = NULL;
    int size = 0;
    uint8_t *file_ptr;
    char file_size[FILE_SIZE_LENGTH + 1];
    unsigned int i, file_len, write_len, session_done, file_done, packets_received, errors = 0;
    int packet_length = 0;
    file_len = 0;
    int eof_cnt = 0;

    #if CONFIG_JOLT_COMPRESSION_AUTO 
    static decomp_t *d = NULL;
    #endif

    char name[JOLT_FS_MAX_FILENAME_BUF_LEN];
    if( NULL == getname ){
        getname = name;
    }

    vTaskPrioritySet(NULL, 17);

    /* Profiling Variables */
#if CONFIG_JOLT_BT_YMODEM_PROFILING
    uint64_t t_ymodem_start = 0, t_ymodem_end = 0;
    uint64_t t_disk = 0;
    t_ymodem_send = 0;
    t_ymodem_receive = 0;
    t_ymodem_start = esp_timer_get_time();
#endif

    packet_data = malloc(PACKET_1K_SIZE + PACKET_OVERHEAD);
    if(NULL == packet_data){
        goto exit;
    }

    jolt_cli_suspend();

    send_CRC16(); /* Initiate Transfer */
    
    for (session_done = 0, errors = 0; ;) {
        for (packets_received = 0, file_done = 0; ;) {
            if(size > 0 && NULL != progress) {
                /* update progress value */
                *progress = file_len * 100 / size;
            }
            switch (receive_packet(packet_data, &packet_length, NAK_TIMEOUT)) {
                case 0:    // normal return
                    switch (packet_length) {
                        case -1:
                            // Abort by sender
                            SEND_ACK_EXIT(-1);
                        case -2:
                            BLE_UART_LOGW("%d) Case -2", __LINE__);
                            /* Fall Through */
                        case -3:
                            // error
                            BLE_UART_LOGW("%d) Case -3", __LINE__);
                            
                            errors ++;
                            if (errors > 5) {
                                SEND_CA_EXIT(-2);
                            }
                            send_NAK();
                            break;
                        case 0:
                            // End of transmission
                            eof_cnt++;
                            BLE_UART_LOGI("%d) EOT %d", __LINE__, eof_cnt);
                            if (eof_cnt == 1) {
                                send_NAK();
                            }
                            else {
                                send_ACKCRC16();
                            }
                            break;
                        default:
                            // ** Normal packet **
                            if (eof_cnt > 1) {
                                send_ACK();
                            }
                            else if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) != (packets_received & 0x000000ff)) {
                                BLE_UART_LOGE("%d) Incorrect PACKET_SEQNO %02x; expected %02x", __LINE__,
                                        packet_data[PACKET_SEQNO_INDEX] & 0xff, packets_received & 0x000000ff);
                                errors ++;
                                if (errors > MAX_ERRORS) {
                                    SEND_CA_EXIT(-3);
                                }
                                send_NAK();
                            }
                            else {
                                if (packets_received == 0) {
#if CONFIG_JOLT_BT_YMODEM_PROFILING
                                    ymodem_transfer_in_progress = true;
#endif
                                    // ** First packet, Filename packet **
                                    if (packet_data[PACKET_HEADER] != 0) {
                                        errors = 0;
                                        // ** Filename packet has valid data
                                        {
                                            char *name = getname;
                                            for (i = 0, file_ptr = packet_data + PACKET_HEADER;
                                                    *file_ptr != 0; i++) {
                                                if(i >= JOLT_FS_MAX_FILENAME_LEN){
                                                    /* Filename too long */
                                                    size = -13;
                                                    goto exit;
                                                }
                                                *name++ = *file_ptr++;
                                            }
                                            *name = '\0';
                                            #if CONFIG_JOLT_COMPRESSION_AUTO 
                                            /* Check if the suffix is ".gz" */
                                            if( 0 == strcmp( name-3, ".gz" ) ) {
                                                d = decompress_obj_init(write_fun, ffd);
                                                if(NULL == d){
                                                    SEND_CA_EXIT(-12);
                                                }
                                                /* remove the ".gz" suffix */
                                                name -=3;
                                                *name = '\0';
                                            }
                                            #endif
                                        }
                                        for (i = 0, file_ptr = packet_data + PACKET_HEADER;
                                                (*file_ptr != 0) && (i < packet_length);) {
                                            file_ptr++;
                                        }
                                        for (i = 0, file_ptr++; *file_ptr != ' ' && *file_ptr != '\0';) {
                                            if( i >= FILE_SIZE_LENGTH ) {
                                                /* This file is definitely too big */
                                                size = -9;
                                                goto exit;
                                            }
                                            file_size[i++] = *file_ptr++;
                                        }
                                        file_size[i++] = '\0';
                                        if (strlen(file_size) > 0) size = strtol(file_size, NULL, 10);
                                        else size = 0;
                                        BLE_UART_LOGI("Header indicates file is %d long.\n", size);

                                        // Test the size of the file
                                        if ((size < 1) || (size > maxsize)) {
                                            // End session
                                            send_CA();
                                            if (size > maxsize) {
                                                size = -9;
                                            }
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
                                        errors ++;
                                        if (errors > 5) {
                                            SEND_CA_EXIT(-5);
                                        }
                                        send_NAK();
                                    }
                                }
                                else {
                                    // ** Data packet **
                                    send_ACK();
                                    BLE_UART_LOGD("%d) Received Data Packet", __LINE__);
                                    // Write received data to file
                                    if (file_len < size) {
                                        file_len += packet_length;    // total bytes received
                                        if (file_len > size) {
                                            /* Truncate the final packet */
                                            write_len = packet_length - (file_len - size);
                                            file_len = size;
                                        }
                                        else {
                                            write_len = packet_length;
                                        }

                                        int written_bytes; 
                                        uint64_t t_disk_start = esp_timer_get_time();
                                        #if CONFIG_JOLT_COMPRESSION_AUTO 
                                        if( NULL != d ){
                                            decompress_obj_chunk( d,
                                                            (uint8_t*)(packet_data + PACKET_HEADER), 
                                                            write_len );
                                            written_bytes = write_len; // tmp workaround
                                        }
                                        else
                                        #endif
                                        {
#if 1
                                            BLE_UART_LOGD("Writing %d bytes to disk.\n", write_len);
                                            written_bytes = write_fun(
                                                    (char*)(packet_data + PACKET_HEADER), 
                                                    1, write_len, ffd);
                                            BLE_UART_LOGD("%d bytes written to disk.\n", file_len);
#else
                                            written_bytes = write_len;
#endif
                                        }
                                        t_disk += esp_timer_get_time() - t_disk_start;

                                        if (written_bytes != write_len) { //failed
                                            /* End session */
                                            SEND_CA_EXIT(-6);
                                        }
                                    }
                                    //success
                                    errors = 0;
                                    taskYIELD();
                                }
                                packets_received++;
                            }
                    }
                    break;
                case ABORT_BY_USER:    // user abort
                    SEND_CA_EXIT(-7);
                default: // timeout
                    BLE_UART_LOGD("%d) TIMEOUT", __LINE__);
                    if (eof_cnt > 1) {
                        file_done = 1;
                    }
                    else {
                        errors ++;
                        if (errors > MAX_ERRORS) {
                            SEND_CA_EXIT(-8);
                        }
                        else if(size == 0){
                            send_CRC16();
                        }
                        else{
                            send_NAK();
                        }
                    }
            }
            if (file_done != 0) {
                session_done = 1;
                break;
            }
        }
        if (session_done != 0) {
            break;
        }
    }

exit:

#if CONFIG_JOLT_BT_YMODEM_PROFILING
    t_ymodem_end = esp_timer_get_time();
#endif
#if CONFIG_JOLT_COMPRESSION_AUTO 
    decompress_obj_del( d );
#endif

    vTaskPrioritySet(NULL, CONFIG_JOLT_TASK_PRIORITY_BACKGROUND);

    jolt_cli_resume();
    esp_log_level_set("*", CONFIG_LOG_DEFAULT_LEVEL);
    esp_log_level_set("wifi", ESP_LOG_NONE);
    if(NULL != packet_data) {
        free(packet_data);
    }
    BLE_UART_LOGI("\n%d errors corrected during transfer.", errors);
    if( NULL != progress && size < 0) {
        *progress = -1;
    }
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

    if(size <= 0) BLE_UART_LOGE("Error %d", size);

#if CONFIG_JOLT_BT_YMODEM_PROFILING
    BLE_UART_LOGI(
            "\n-----------------------------\n"
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
            size,
            (uint32_t) t_ymodem_receive,
            (uint32_t) t_ymodem_send,
            (uint32_t) t_disk,
#if CONFIG_JOLT_BT_PROFILING
            (uint32_t) (ble_packet_cum_life / ble_packet_n),
#endif
            (uint32_t) (t_ymodem_packet_cum / n_ymodem_packet),
            (uint32_t) (t_ymodem_first_byte_cum / n_ymodem_first_byte),
            (uint32_t) t_ble_read_timeout,
            (uint32_t) (t_ymodem_end - t_ymodem_start));
#endif

    return size;
}

// Receive a file using the ymodem protocol.
//-----------------------------------------------------------------
int IRAM_ATTR ymodem_receive (FILE *ffd, unsigned int maxsize, char* getname, int8_t *progress) {
    return ymodem_receive_write(ffd, maxsize, getname, (write_fun_t)&fwrite, progress);
}

