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

#include <driver/uart.h>
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
#include "syscore/ymodem.h"
#include "syscore/ymodem/ymodem_common.h"

//------------------------------------------------------------------------------------
static void ymodem_prepare_initial_packet( uint8_t *data, char *fileName, uint32_t length )
{
    uint16_t tempCRC;

    memset( data, 0, PACKET_SIZE + PACKET_HEADER );
    // Make first three packet
    data[0] = SOH;
    data[1] = 0x00;
    data[2] = 0xff;

    // add filename
    sprintf( (char *)( data + PACKET_HEADER ), "%s", fileName );

    // add file site
    sprintf( (char *)( data + PACKET_HEADER + strlen( (char *)( data + PACKET_HEADER ) ) + 1 ), "%u", length );
    data[PACKET_HEADER + strlen( (char *)( data + PACKET_HEADER ) ) + 1 +
         strlen( (char *)( data + PACKET_HEADER + strlen( (char *)( data + PACKET_HEADER ) ) + 1 ) )] = ' ';

    // add crc
    tempCRC                               = crc16( &data[PACKET_HEADER], PACKET_SIZE );
    data[PACKET_SIZE + PACKET_HEADER]     = tempCRC >> 8;
    data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-------------------------------------------------
static void ymodem_prepare_last_packet( uint8_t *data )
{
    uint16_t tempCRC;

    memset( data, 0, PACKET_SIZE + PACKET_HEADER );
    data[0]                               = SOH;
    data[1]                               = 0x00;
    data[2]                               = 0xff;
    tempCRC                               = crc16( &data[PACKET_HEADER], PACKET_SIZE );
    data[PACKET_SIZE + PACKET_HEADER]     = tempCRC >> 8;
    data[PACKET_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-----------------------------------------------------------------------------------------
static void ymodem_prepare_packet( uint8_t *data, uint8_t pktNo, uint32_t sizeBlk, FILE *ffd )
{
    uint16_t size, tempCRC;

    data[0] = STX;
    data[1] = ( pktNo & 0x000000ff );
    data[2] = ( ~( pktNo & 0x000000ff ) );

    size = sizeBlk < PACKET_1K_SIZE ? sizeBlk : PACKET_1K_SIZE;
    // Read block from file
    if( size > 0 ) { size = fread( data + PACKET_HEADER, 1, size, ffd ); }

    if( size < PACKET_1K_SIZE ) {
        for( uint16_t i = size + PACKET_HEADER; i < PACKET_1K_SIZE + PACKET_HEADER; i++ ) {
            data[i] = 0x00;  // EOF (0x1A) or 0x00
        }
    }
    tempCRC = crc16( &data[PACKET_HEADER], PACKET_1K_SIZE );
    // tempCRC = crc16_le(0, &data[PACKET_HEADER], PACKET_1K_SIZE);
    data[PACKET_1K_SIZE + PACKET_HEADER]     = tempCRC >> 8;
    data[PACKET_1K_SIZE + PACKET_HEADER + 1] = tempCRC & 0xFF;
}

//-------------------------------------------------------------
static uint8_t ymodem_wait_response( uint8_t ackchr, uint8_t tmo )
{
    unsigned char receivedC;
    uint32_t errors = 0;

    do {
        if( receive_byte( &receivedC, NAK_TIMEOUT ) == 0 ) {
            if( receivedC == ackchr ) { return 1; }
            else if( receivedC == CA ) {
                send_CA();
                return 2;  // CA received, Sender abort
            }
            else if( receivedC == NAK ) {
                return 3;
            }
            else {
                return 4;
            }
        }
        else {
            errors++;
        }
    } while( errors < tmo );
    return 0;
}

//------------------------------------------------------------------------
#undef SEND_CA_EXIT
#define SEND_CA_EXIT( x ) \
    send_CA();            \
    error_code = x;       \
    goto exit;
int ymodem_transmit( char *sendFileName, unsigned int sizeFile, FILE *ffd )
{
    int error_code = 0;
    uint8_t *packet_data;

    uint16_t blkNumber;
    unsigned char receivedC;
    int err;
    uint32_t size = 0;

    packet_data = malloc( PACKET_1K_SIZE + PACKET_OVERHEAD );
    if( NULL == packet_data ) {
        error_code = -20;
        goto exit;
    }

    rx_consume();

    // Wait for response from receiver
    err = 0;
    do {
        send_byte( CRC16 );
    } while( receive_byte( &receivedC, NAK_TIMEOUT ) < 0 && err++ < 45 );

    if( err >= 45 ) { SEND_CA_EXIT( -1 ); }
    else if( receivedC != CRC16 ) {
        SEND_CA_EXIT( -21 );
    }

    // === Prepare first block and send it =======================================
    /* When the receiving program receives this block and successfully
     * opened the output file, it shall acknowledge this block with an ACK
     * character and then proceed with a normal YMODEM file transfer
     * beginning with a "C" or NAK tranmsitted by the receiver.
     */
    ymodem_prepare_initial_packet( packet_data, sendFileName, sizeFile );
    do {
        // Send Packet
        send_bytes( (char *)packet_data, PACKET_SIZE + PACKET_OVERHEAD );

        // Wait for Ack
        err = ymodem_wait_response( ACK, 10 );
        if( err == 0 || err == 4 ) {
            // timeout or wrong response
            SEND_CA_EXIT( -2 );
        }
        else if( err == 2 ) {
            /* Abort */
            error_code = 98;
            goto exit;
        }
    } while( err != 1 );

    // After initial block the receiver sends 'C' after ACK
    if( ymodem_wait_response( CRC16, 10 ) != 1 ) { SEND_CA_EXIT( -3 ); }

    // === Send file blocks ======================================================
    size      = sizeFile;
    blkNumber = 0x01;

    // Resend packet if NAK    for a count of 10 else end of communication
    while( size ) {
        // Prepare and send next packet
        ymodem_prepare_packet( packet_data, blkNumber, size, ffd );
        do {
            send_bytes( (char *)packet_data, PACKET_1K_SIZE + PACKET_OVERHEAD );

            // Wait for Ack
            err = ymodem_wait_response( ACK, 10 );
            if( err == 1 ) {
                blkNumber++;
                if( size > PACKET_1K_SIZE )
                    size -= PACKET_1K_SIZE;  // Next packet
                else
                    size = 0;  // Last packet sent
            }
            else if( err == 0 || err == 4 ) {
                // timeout or wrong response
                SEND_CA_EXIT( -4 );
            }
            else if( err == 2 ) {
                /* Abort */
                error_code = -5;
                goto exit;
            }
        } while( err != 1 );
    }

    // === Send EOT ==============================================================
    send_byte( EOT );  // Send (EOT)
    // Wait for Ack
    do {
        // Wait for Ack
        err = ymodem_wait_response( ACK, 10 );
        if( err == 3 ) {       // NAK
            send_byte( EOT );  // Send (EOT)
        }
        else if( err == 0 || err == 4 ) {
            // timeout or wrong response
            SEND_CA_EXIT( -6 );
        }
        else if( err == 2 ) {
            /* Abort */
            error_code = -7;
            goto exit;
        }
    } while( err != 1 );

    // === Receiver requests next file, prepare and send last packet =============
    if( ymodem_wait_response( CRC16, 10 ) != 1 ) { SEND_CA_EXIT( -8 ); }

    ymodem_prepare_last_packet( packet_data );
    do {
        // Send Packet
        uart_write_bytes( EX_UART_NUM, (char *)packet_data, PACKET_SIZE + PACKET_OVERHEAD );

        // Wait for Ack
        err = ymodem_wait_response( ACK, 10 );
        if( err == 0 || err == 4 ) { SEND_CA_EXIT( -9 ); }
        else if( err == 2 ) {
            error_code = -10;
            goto exit;
        }
    } while( err != 1 );

    // success!
    error_code = 0;

exit:
    if( NULL != packet_data ) { free( packet_data ); }

    return error_code;  // file transmitted successfully
}
