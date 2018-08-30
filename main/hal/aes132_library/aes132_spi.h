// ----------------------------------------------------------------------------
//         ATMEL Crypto-Devices Software Support  -  Colorado Springs, CO -
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
 *  \brief 	This file contains definitions of the SPI layer of the AES132 library.
 *  \author Atmel Crypto Products
 *  \date 	June 16, 2011
 */

#ifndef AES132_SPI_H
#   define AES132_SPI_H

#include <stdint.h>


// ----------------------------------------------------------------------------------
// ------------------------ definitions for timeout counters ------------------------
// ----------- Adjust the values for the timeout counters to your hardware. ---------
// ----------------------------------------------------------------------------------

/** \brief time for polling a bit in the device status register in us (measured)
 *
 * With an oscilloscope or logic analyzer, measure the time it takes for one
 * loop iteration in aes132c_wait_for_status_bit() and enter it here.
 * Undefine this if you want to use estimated values (see below).
 */
#define AES132_STATUS_REG_POLL_TIME            (23)

#ifndef AES132_STATUS_REG_POLL_TIME

/** \brief time in us per byte (number of clocks per byte * number of bytes / clock frequency)
 *
 * If you cannot measure the polling time for a status bit in the device status register,
 * you can derive it by establishing three separate values,
 * #AES132_STATUS_REG_POLL_TIME_COMM, #AES132_STATUS_REG_POLL_TIME_OVERHEAD,
 * and #AES132_STATUS_REG_POLL_TIME_LOOP.
 *
 * Time to poll the device status register:\n
 *    t_poll = t_comm + t_comm_overhead + t_loop,\n
 * where\n
 *    t_comm = (4 bytes * 8 clocks / spi_clock)\n
 *    t_comm_overhead = t_data_register_write * 3 + t_data_register_read,\n
 *    t_loop = t_loop(aes132c_read_device_status_register())\n
 *
 * Example for AVR eight-bit CPU running at 16 MHz and SPI clocked at 8 MHz:\n
 *    t_comm = 32 bits / 8 MHz = 4.0 us\n
 *    t_comm_overhead = 5.8 us\n
 *    t_loop = 13.2 us\n
 */
#define AES132_STATUS_REG_POLL_TIME_COMM         ( 4)

//! time in us it takes to access the interface peripheral
#define AES132_STATUS_REG_POLL_TIME_OVERHEAD     ( 6)

//! time in us the polling loop takes to decrement the counter and branch
#define AES132_STATUS_REG_POLL_TIME_LOOP         (13)

/** \brief time for polling a bit in the device status register in us (calculated)

 * Use either the definition where the value was measured
 * or this one where individual values are calculated and / or estimated.
 */
#   define AES132_STATUS_REG_POLL_TIME (AES132_STATUS_REG_POLL_TIME_COMM + \
			AES132_STATUS_REG_POLL_TIME_OVERHEAD + AES132_STATUS_REG_POLL_TIME_LOOP)
#endif

//! timeout loop iterations per ms derived from the times it takes to communicate and loop when acked
#define AES132_ITERATIONS_PER_MS      (1000 / AES132_STATUS_REG_POLL_TIME)

//! Poll this many times for the device being ready for access.
#define AES132_RETRY_COUNT_DEVICE_READY   ((uint16_t) (AES132_DEVICE_READY_TIMEOUT * AES132_ITERATIONS_PER_MS))

//! Poll this many times for the response buffer being ready for reading.
#define AES132_RETRY_COUNT_RESPONSE_READY ((uint16_t) (AES132_RESPONSE_READY_TIMEOUT * AES132_ITERATIONS_PER_MS * 2))

// ----------------------------------------------------------------------------------
// --------------- end of definitions for timeout counter values --------------------
// ----------------------------------------------------------------------------------

// ------------ definitions for library return codes ----------------------------

#define AES132_FUNCTION_RETCODE_ADDRESS_WRITE_NACK   ((uint8_t) 0xA0) //!< I2C nack when sending an I2C address for writing (only returned by I2C)
#define AES132_FUNCTION_RETCODE_ADDRESS_READ_NACK    ((uint8_t) 0xA1) //!< I2C nack when sending an I2C address for reading (only returned by I2C)
#define AES132_FUNCTION_RETCODE_SIZE_TOO_SMALL       ((uint8_t) 0xA2) //!< Count value in response was bigger than buffer.

// The codes below are the same as in the SHA204 library.
#define AES132_FUNCTION_RETCODE_SUCCESS              ((uint8_t) 0x00) //!< Function succeeded.
#define AES132_FUNCTION_RETCODE_BAD_CRC_TX           ((uint8_t) 0xD4) //!< Device status register bit 4 (CRC) is set.
#define AES132_FUNCTION_RETCODE_NOT_IMPLEMENTED      ((uint8_t) 0xE0) //!< interface function not implemented
#define AES132_FUNCTION_RETCODE_DEVICE_SELECT_FAIL   ((uint8_t) 0xE3) //!< device index out of bounds
#define AES132_FUNCTION_RETCODE_COUNT_INVALID        ((uint8_t) 0xE4) //!< count byte in response is out of range
#define AES132_FUNCTION_RETCODE_BAD_CRC_RX           ((uint8_t) 0xE5) //!< incorrect CRC received
#define AES132_FUNCTION_RETCODE_TIMEOUT              ((uint8_t) 0xE7) //!< Function timed out while waiting for response.
#define AES132_FUNCTION_RETCODE_COMM_FAIL            ((uint8_t) 0xF0) //!< Communication with device failed.


void    aes132p_enable_interface(void);
void    aes132p_disable_interface(void);
uint8_t aes132p_select_device(uint8_t device_id);
uint8_t aes132p_read_memory_physical(uint8_t size, uint16_t word_address, uint8_t *data);
uint8_t aes132p_write_memory_physical(uint8_t count, uint16_t word_address, uint8_t *data);
uint8_t aes132p_resync_physical(void);

#endif
