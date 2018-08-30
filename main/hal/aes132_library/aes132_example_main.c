// ----------------------------------------------------------------------------
//         ATMEL Crypto_Devices Software Support  -  Colorado Springs, CO -
// ----------------------------------------------------------------------------
// DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
// DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------
/** \file
 *  \brief This file contains the main function for AES132 library example.
 *  \author Atmel Crypto Products
 *  \date June 16, 2015
 */
#include <asf.h>
#include <stdint.h>
#include <string.h>

#include "stdio_serial.h"
#include "aes132_comm_marshaling.h"

// Macros
#define PACKET_BLOCK_SIZE_MAX    (256)

static struct usart_module cdc_uart_module;
uint16_t word_address = 0x0000;
uint16_t rx_byte_count = 0x0004;
uint8_t tx_buffer_write[] = {0x55, 0xAA, 0xBC, 0xDE}; // Data to be written
uint8_t aes132_lib_return, serial_data;

// Global variables for SPI/I2C interface
uint8_t tx_buffer[84];
uint8_t rx_buffer[36];

// Global variables for serial
uint8_t  g_packet_block[PACKET_BLOCK_SIZE_MAX];
uint16_t g_packet_block_index;


/** \brief Converting Nibble to ASCII hex
 *
 * \param data is nibble data to be converted
 *
 * \return data is the ASCII hex value
**/
uint8_t nibble_to_hex(uint8_t data)
{
	data &= 0x0F;
	if (data <= 0x09) {
		// 0x0-0x9
		data += '0';
	} else {
		// 0xA-0xF
		data = data - 10 + 'A';
	}
	return data;
}

/** \brief Converting ASCII hex to nibble
 *
 * \param data is the ASCII hex value to be converted
 *
 * \return data is the nibble value
**/
uint8_t hex_to_nibble(uint8_t data)
{
	if ((data >= '0') && (data <= '9' )) {
		// Numbers (0-9)
		data -= '0';
	} else if ((data >= 'A') && (data <= 'F' )) {
		// Uppercase (A-F)
		data = data -'A' + 10;
	} else if ((data >= 'a') && (data <= 'f' )) {
		// Lowercase (a-f)
		data = data -'a' + 10;
	} else {
		// Illegal
		data = 0;
	}
	
	return data;
}

/** \brief print array to UART
 *
 * \param buffer is array which will be printed to UART
 *
 * \param ucLength is array size
**/
void printf_puthex_array(uint8_t* data_buffer, uint8_t length)
{
	uint8_t i_data;
	
	for (i_data = 0; i_data < length; i_data++) {
		printf("%.2X",*data_buffer++);
		printf(" ");
	}
}

static void configure_console(void)
{
	struct usart_config usart_conf;

	usart_get_config_defaults(&usart_conf);
	usart_conf.mux_setting = EDBG_CDC_SERCOM_MUX_SETTING;
	usart_conf.pinmux_pad0 = EDBG_CDC_SERCOM_PINMUX_PAD0;
	usart_conf.pinmux_pad1 = EDBG_CDC_SERCOM_PINMUX_PAD1;
	usart_conf.pinmux_pad2 = EDBG_CDC_SERCOM_PINMUX_PAD2;
	usart_conf.pinmux_pad3 = EDBG_CDC_SERCOM_PINMUX_PAD3;
	usart_conf.baudrate    = 115200;

	stdio_serial_init(&cdc_uart_module, EDBG_CDC_MODULE, &usart_conf);
	usart_enable(&cdc_uart_module);
}

/** \brief Parses a byte from the argument, put the parsed data into a global array.
 *         Uses static variables and global variables.
 *         Set #PACKET_BLOCK_SIZE_MAX define in parser.h to maximum buffer size needed.
 *
 *  \param serial_data Data input
 *  \return 1 on complete parsing, 0 otherwise
 */
uint8_t parse_command(uint8_t serial_data)
{
	static unsigned char parser_state, nibble_value;
	
	switch (parser_state) {
		case 0:
			g_packet_block_index = 0;
			if (serial_data == '(') {
				parser_state = 1;
			} else {
				printf("Input :%c",serial_data);
			}
			break;
		case 1:
			nibble_value = hex_to_nibble(serial_data);
			parser_state = 2;
			break;
		case 2:
			g_packet_block[g_packet_block_index] = (nibble_value << 4) + hex_to_nibble(serial_data);
			if (g_packet_block_index < PACKET_BLOCK_SIZE_MAX - 1) {
				g_packet_block_index++;
			} else {
				printf("Error buffer overflow");
			}
			parser_state = 3;
			break;
		case 3:
			if (serial_data == ' ') {
				parser_state = 1;
			} else if (serial_data == ')') {
				parser_state = 0;
				printf("\n");
				return 1;
			} else {
				parser_state = 0;
			}
			break;
		default:
			parser_state = 0;
	} // switch (parser_state)
	
	return 0;
}





// Write memory test
uint8_t write_memory_test(void)
{
	// -------------------- Write memory. -----------------------------------
	// Don't put this in an infinite loop. Otherwise the non-volatile memory will wear out.
	aes132_lib_return = aes132m_write_memory(sizeof(tx_buffer_write), word_address, tx_buffer_write);
	if (aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS) {
		printf("Write memory failed!\r\n");
		return aes132_lib_return;
	}
	printf("Write memory success!\r\n");
	printf("Data: ");
	printf_puthex_array(tx_buffer_write,sizeof(tx_buffer_write));
	printf("\r\n");
}

// Read memory test
uint8_t read_memory_test(void)
{
	// -------------------- Read memory. -----------------------------------
	aes132_lib_return = aes132m_read_memory(rx_byte_count, word_address, rx_buffer);
	if (aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS) {
		printf("Read memory failed!\r\n");
		return aes132_lib_return;
	}
	printf("Read memory success!\r\n");
	printf("Data: ");
	printf_puthex_array(rx_buffer,sizeof(tx_buffer_write));
	printf("\r\n");
	// -------------------- Compare written with read data. -----------------------------------
	aes132_lib_return =  memcmp(tx_buffer_write, rx_buffer, sizeof(tx_buffer_write));
	if (aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS) {
		printf("Compare Memory mismatch!\r\n");
		return aes132_lib_return;
	}
	printf("Compare Memory match!\r\n");
}

// Send a BlockRead command
uint8_t block_read_test(void)
{
	// ------- Send a BlockRead command and receive its response. -----------------------------
	aes132_lib_return = aes132m_execute(AES132_BLOCK_READ, 0, word_address, rx_byte_count,
	0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
	if (aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS) {
		printf("Block Read memory failed!\r\n");
		return aes132_lib_return;
	}
	printf("Block Read memory success!\r\n");
	printf("Data: ");
	printf_puthex_array(&rx_buffer[AES132_RESPONSE_INDEX_DATA],rx_byte_count);
	
	// -------------------- Compare written with read data. -----------------------------------
	aes132_lib_return =  memcmp(tx_buffer_write, &rx_buffer[AES132_RESPONSE_INDEX_DATA], sizeof(tx_buffer_write));
	if (aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS) {
		printf("Compare Memory mismatch!\r\n");
		return aes132_lib_return;
	}
	printf("Compare Memory match!\r\n");
}


void aes132app_single_execute(void)
{
	uint8_t* p_packet_block = g_packet_block;
	
	uint8_t  op_code  = 0;
	uint8_t  mode	  = 0;
	uint16_t param1   = 0;
	uint16_t param2   = 0;
	uint8_t  datalen1 = 0;
	uint8_t* data1    = NULL;
	uint8_t  datalen2 = 0;
	uint8_t* data2    = NULL;
	uint8_t  datalen3 = 0;
	uint8_t* data3    = NULL;
	uint8_t  datalen4 = 0;
	uint8_t* data4    = NULL;
	uint8_t  ret_code = 0xFF;

	// 4 bytes Opcode, Param1, Param2
	op_code  =  *p_packet_block++;
	mode	 =	*p_packet_block++;
	param1   = (*p_packet_block++ << 8);
	param1  |= (*p_packet_block++);
	param2   = (*p_packet_block++ << 8);
	param2  |= (*p_packet_block++);
	
	// The rest is included as Data1
	datalen1 = g_packet_block_index - 6;
	data1    = p_packet_block;
	
	// Execute
	ret_code = aes132m_execute(op_code,mode, param1, param2,
	datalen1, data1, datalen2, data2, datalen3, data3,
	datalen4, data4, tx_buffer, rx_buffer);
	printf("\n");
	printf("TX buffer = 0x");
	printf_puthex_array(tx_buffer, tx_buffer[0]);
	printf("\n");
	if(ret_code == AES132_FUNCTION_RETCODE_SUCCESS){
		printf("RX buffer = 0x");
		printf_puthex_array(rx_buffer, rx_buffer[0]);
		printf("\n");
	}else
		printf("Failed! ret_code = 0x%.2X\n", ret_code);
	
}


void aes132app_command_dispatch(void)
{
	// Dispatch appropriate function, deduce from the opcode (packet block byte #0)
	// Use dummy opcodes for specific cases
	switch (g_packet_block[0]) {
		case 0x80:
			write_memory_test();
		break;
		case 0x81:
			read_memory_test();
		break;
		case 0x82:
			block_read_test();
		break;
		default:
			aes132app_single_execute();
		break;
	}
}

/** \brief This function is the entry function for an example application that
           uses the AES132 library.
 * \return result (0: success, otherwise failure)
 */
int main(void)
{
	//! [system_init]
	system_init();
	//! [system_init]
	
	//! [configure_console]
	configure_console();
	//! [configure_console]
	
	//! [aes132p_enable_interface]
	aes132p_enable_interface();
	//! [aes132p_enable_interface]
	printf("*****AES132 TEST*****\r\n");
	while(1){
		if (scanf("%c",&serial_data)>0){
			if (parse_command(serial_data))
				aes132app_command_dispatch();
		};
	}
	aes132p_disable_interface();

	return aes132_lib_return;
}

