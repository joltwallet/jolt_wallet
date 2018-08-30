/*
 * swi_phys.h
 *
 * Created: 6/8/2015 1:41:37 PM
 *  Author: Atmel Baliteam
 */ 


#ifndef SWI_PHYS_H_
#define SWI_PHYS_H_

#include <asf.h>
#include <stdint.h>				// data type definitions
#include "aes132_spi.h"
// ----------------------------------------------------------------------------------
// ---- definitions and declarations of ports and pins CS\ pins are connected to ----
// ---------------------- Modify these to match your hardware. ----------------------
// ----------------------------------------------------------------------------------

/** \brief Adjust this variable considering how long one iteration of
 *         the inner while loop in spi_receive_bytes() takes
 *         (receiving one byte and decrementing the timeout counter).
 */
#define SPI_RX_TIMEOUT    ((uint8_t) 250)

/** \brief Adjust this variable considering how long one iteration of
 *         the inner while loop in spi_send_bytes() takes,
 *         (sending one byte and decrementing the timeout counter).
 */
#define SPI_TX_TIMEOUT    ((uint8_t)  50)

#define SPI_DEVICE_COUNT  (1)     //!< number of AES132 devices

#define SLAVE_SELECT_PIN EXT3_PIN_SPI_SS_1

//! array of SPI chip select pins
const static uint8_t spi_select[SPI_DEVICE_COUNT] = {SLAVE_SELECT_PIN};

/** \brief This variable holds the index for SPI port
 *         and chip select pin currently in use. Default is index 0.
 */
static uint8_t spi_current_index = 0;

// ----------------------------------------------------------------------------------
// ------------- end of ports and pins definitions and declarations -----------------
// ----------------------------------------------------------------------------------


// error codes for physical hardware dependent module
// Codes in the range 0x00 to 0xF7 are shared between physical interfaces (I2C, SPI).
// Codes in the range 0xF8 to 0xFF are special for the particular interface.
#define SPI_FUNCTION_RETCODE_SUCCESS     ((uint8_t) 0x00) //!< Communication with device succeeded.
#define SPI_FUNCTION_RETCODE_COMM_FAIL   ((uint8_t) 0xF0) //!< Communication with device failed.
#define SPI_FUNCTION_RETCODE_TIMEOUT     ((uint8_t) 0xF1) //!< Communication timed out.


// Function prototypes to be implemented in the target spi_phys.c
void    spi_enable_phys(void);
void    spi_disable_phys(void);
void    spi_select_slave_phys(void);
void    spi_deselect_slave_phys(void);
uint8_t spi_select_device(uint8_t index);
uint8_t spi_send_bytes(uint8_t count, uint8_t *data);
uint8_t spi_receive_bytes(uint8_t count, uint8_t *data);

#endif /* SWI_PHYS_H_ */