/**
 * @file ymodem_common.h
 * @brief Common YMODEM functionality for transmits and receives
 * 
 * Should only be included in the YMODEM source files.
 *
 * @author Brian Pugh, Boris Lovosevic
 */

#ifndef YMODEM_COMMON_H__
#define YMODEM_COMMON_H__

#include "jolt_helpers.h"
#include "sdkconfig.h"
#include <esp_timer.h>

/* Don't use this header outside of the ymodem files */
#define ABORT_BY_USER -2
#define ABORT_BY_TIMEOUT -1

// === UART DEFINES ====
#define EX_UART_NUM UART_NUM_0

// ==== Y-MODEM defines ====
#define PACKET_SEQNO_INDEX      (1)
#define PACKET_SEQNO_COMP_INDEX (2)

#define PACKET_HEADER           (3)
#define PACKET_TRAILER          (2)
#define PACKET_OVERHEAD         (PACKET_HEADER + PACKET_TRAILER)
#define PACKET_SIZE             (128)
#define PACKET_1K_SIZE          (1024)

#define FILE_SIZE_LENGTH        (7) /* Maximum number of ascii digits file length can be */

#define SOH                     (0x01)  /* start of 128-byte data packet (Start of Header) */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* End of Transmission */
#define ACK                     (0x06)  /* Acknowledge */
#define NAK                     (0x15)  /* Negative Acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer (CAncel)*/
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             (1000)
#define MAX_ERRORS              (45)

#if CONFIG_JOLT_BT_YMODEM_PROFILING
extern uint64_t t_ymodem_send, t_ymodem_receive;
extern uint64_t t_ble_read_timeout;
extern bool ymodem_transfer_in_progress;
#endif

/**
 * @brief compute the CRC16 checksum
 * @param[in] buf Input data buffer
 * @param[in] count Input data buffer length in bytes
 * @return CRC16 checksum
 */
unsigned short IRAM_ATTR crc16(const unsigned char *buf, unsigned long count);

/**
 * @brief Wrapper to read bytes from STDIN
 * @param[out] c Data buffer to store received bytes
 * @param[in] timeout maximum number of milliseconds to wait before returning an incomplete buffer
 * @param[in] count Number of bytes to read.
 * @reutrn Number of bytes read
 */
int32_t IRAM_ATTR receive_bytes (unsigned char *c, uint32_t timeout, uint32_t n);

/**
 * @brief Wrapper to read a single byte from STDIN
 * @param[out] c character buffer
 * @param[in] timeout maximum number of milliseconds to wait before returning without a byte
 * @return Number of bytes read
 */
static inline int32_t IRAM_ATTR receive_byte (unsigned char *c, uint32_t timeout) {
    return receive_bytes(c, timeout, 1);
}

/**
 * @brief Flush STDIN
 */
void IRAM_ATTR rx_consume();

/**
 * @brief Wrapper to write bytes to STDOUT
 * @param[in] c pointer to byte array to send
 * @param[in] n number of bytes to send
 */
static inline uint32_t IRAM_ATTR send_bytes(char *c, uint32_t n) {
    const char new_line = '\n';
    uint64_t t_start = esp_timer_get_time();
    fwrite(c, 1, n, stdout);
    fwrite(&new_line, 1, 1, stdout);
    t_ymodem_send += esp_timer_get_time() - t_start; 
    return 0;
}

/**
 * @brief Wrapper to send a single byte to STDOUT
 * @param[in] c byte to send
 */
static inline uint32_t IRAM_ATTR send_byte (char c) {
    return send_bytes(&c, 1);
}

/**
 * @brief Helper to send Cancel bytes
 */
static inline void IRAM_ATTR send_CA ( void ) {
    send_byte(CA);
    send_byte(CA);
}

/**
 * @brief Helper to send Acknowledge byte
 */
static inline void IRAM_ATTR send_ACK ( void ) {
    send_byte(ACK);
}

/**
 * @brief Helper to send Negative Acknowledge byte
 */
static inline void IRAM_ATTR send_NAK ( void ) {
    BLE_UART_LOG("%d) send_NAK", __LINE__);
    send_byte(NAK);
}

/**
 * @brief Helper to send CRC16 byte
 */
static inline void IRAM_ATTR send_CRC16 ( void ) {
    send_byte(CRC16);
}

/**
 * @brief Helper to send Acknowledge byte followed by the CRC16 byte.
 */
static inline void IRAM_ATTR send_ACKCRC16 ( void ) {
    send_ACK();
    send_CRC16();
}


#endif
