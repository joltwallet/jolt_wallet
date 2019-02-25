#ifndef YMODEM_COMMON_H__
#define YMODEM_COMMON_H__

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

#define FILE_SIZE_LENGTH        (16)

#define SOH                     (0x01)  /* start of 128-byte data packet */
#define STX                     (0x02)  /* start of 1024-byte data packet */
#define EOT                     (0x04)  /* end of transmission */
#define ACK                     (0x06)  /* acknowledge */
#define NAK                     (0x15)  /* negative acknowledge */
#define CA                      (0x18)  /* two of these in succession aborts transfer */
#define CRC16                   (0x43)  /* 'C' == 0x43, request 16-bit CRC */

#define ABORT1                  (0x41)  /* 'A' == 0x41, abort by user */
#define ABORT2                  (0x61)  /* 'a' == 0x61, abort by user */

#define NAK_TIMEOUT             (1000)
#define MAX_ERRORS              (45)

unsigned short IRAM_ATTR crc16(const unsigned char *buf, unsigned long count);
int32_t IRAM_ATTR receive_bytes (unsigned char *c, uint32_t timeout, uint32_t n);

inline int32_t IRAM_ATTR receive_byte (unsigned char *c, uint32_t timeout) {
    return receive_bytes(c, timeout, 1);
}

//------------------------
inline void IRAM_ATTR rx_consume() {
    fflush(stdin);
}

//--------------------------------
inline uint32_t IRAM_ATTR send_bytes(char *c, uint32_t n) {
    fwrite(c, 1, n, stdout);
    return 0;
}

inline uint32_t IRAM_ATTR send_byte (char c) {
    return send_bytes(&c, 1);
}

//----------------------------
inline void IRAM_ATTR send_CA ( void ) {
    send_byte(CA);
    send_byte(CA);
}

//-----------------------------
inline void IRAM_ATTR send_ACK ( void ) {
    send_byte(ACK);
}

//----------------------------------
inline void IRAM_ATTR send_ACKCRC16 ( void ) {
    send_byte(ACK);
    send_byte(CRC16);
}

//-----------------------------
inline void IRAM_ATTR send_NAK ( void ) {
    send_byte(NAK);
}

//-------------------------------
inline void IRAM_ATTR send_CRC16 ( void ) {
    send_byte(CRC16);
}


#endif
