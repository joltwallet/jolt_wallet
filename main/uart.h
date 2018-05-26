#ifndef __JOLT_UART_H__
#define __JOLT_UART_H__

void get_serial_input(char *serial_rx, const int buffersize);
void get_serial_input_int(char *serial_rx, const int buffersize);
void flush_uart();

#endif
