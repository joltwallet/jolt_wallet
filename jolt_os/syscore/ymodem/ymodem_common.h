#ifndef YMODEM_COMMON_H__
#define YMODEM_COMMON_H__

/* Don't use this header outside of the ymodem files */
#define ABORT_BY_USER -2
#define ABORT_BY_TIMEOUT -1

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
