/*
 * i2c_phys.h
 *
 * Created: 6/8/2015 1:44:22 PM
 *  Author: Cempaka
 */ 

#ifndef I2C_PHYS_H_
#define I2C_PHYS_H_

#include <stdint.h>				// data type definitions
#include "aes132_i2c.h"
#include "driver/i2c.h"
#include "esp_err.h"

//! I2C clock
#define I2C_CLOCK                         (JOLT_I2C_MASTER_FREQ_HZ)

//! Use pull-up resistors.
#define I2C_PULLUP

/** \brief number of polling iterations for TWINT bit in TWSR after
 *         creating a Start condition in #i2c_send_start()
 *
 * Adjust this value considering how long it takes to check a status bit
 * in the I2C status register, decrement the timeout counter,
 * compare its value with 0, and branch.
 */
#define I2C_START_TIMEOUT                ((uint8_t) 250)

/** \brief number of polling iterations for TWINT bit in TWSR after sending
 *         or receiving a byte.
 *
 * Adjust this value considering how long it takes to check a status bit
 * in the I2C status register, decrement the timeout counter,
 * compare its value with 0, branch, and to send or receive one byte.
 */
#define I2C_BYTE_TIMEOUT                 ((uint8_t) 100)

/** \brief number of polling iterations for TWSTO bit in TWSR after
 *         creating a Stop condition in #i2c_send_stop().
 *
 * Adjust this value considering how long it takes to check a status bit
 * in the I2C control register, decrement the timeout counter,
 * compare its value with 0, and branch.
 */
#define I2C_STOP_TIMEOUT                 ((uint8_t) 250)

// error codes for physical hardware dependent module
// Codes in the range 0x00 to 0xF7 are shared between physical interfaces (SWI, I2C, SPI).
// Codes in the range 0xF8 to 0xFF are special for the particular interface.
#define I2C_FUNCTION_RETCODE_SUCCESS     ((uint8_t) 0x00) //!< Communication with device succeeded.
#define I2C_FUNCTION_RETCODE_COMM_FAIL   ((uint8_t) 0xF0) //!< Communication with device failed.
#define I2C_FUNCTION_RETCODE_TIMEOUT     ((uint8_t) 0xF1) //!< Communication timed out.
#define I2C_FUNCTION_RETCODE_NACK        ((uint8_t) 0xF8) //!< I2C nack

#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */

// Function prototypes to be implemented in the target i2c_phys.c
void    i2c_enable_phys(void);
void    i2c_disable_phys(void);
uint8_t i2c_select_device_phys(uint8_t device_id);
uint8_t i2c_send_bytes(uint8_t count, uint8_t *data);
uint8_t i2c_receive_byte(uint8_t *data, uint8_t *address);
uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data, uint8_t *address);
uint8_t i2c_send_slave_address(uint8_t read);
#endif /* I2C_PHYS_H_ */
