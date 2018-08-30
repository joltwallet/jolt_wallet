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
 *  \brief 	This file contains implementations of SPI functions of the AES132 library.
 *  \author Atmel Crypto Products
 *  \date 	June 16, 2011
 */

#include <stdint.h>
#include "aes132_spi.h"
#include "spi_phys.h"


//! write command id
#define AES132_SPI_WRITE        ((uint8_t) 2)

//! read command id
#define AES132_SPI_READ         ((uint8_t) 3)

//! enable-write command id
#define AES132_SPI_ENABLE_WRITE ((uint8_t) 6)

/** \brief three bytes (command id, word address MSB, word address LSB)
 *         before sending and receiving actual data
 */
#define AES132_SPI_PREFACE_SIZE ((uint8_t) 3)


/** \brief This function initializes and enables the SPI peripheral. */
void aes132p_enable_interface(void)
{
	spi_enable_phys();
}


/** \brief This function disables the SPI peripheral. */
void aes132p_disable_interface(void)
{
	spi_disable_phys();
}


/** \brief This function selects a device from an array of SPI chip select pins
 *         for AES132 devices on the same SPI bus.
 *
 * @param[in] device_id index into the SPI array for chip select ports and or pins
 * @return success or out-of-bounds error
 */
uint8_t aes132p_select_device(uint8_t device_id)
{
	return spi_select_device(device_id);
}


/** \brief This function writes bytes to the device.
 * \param[in] count number of bytes to write
 * \param[in] word_address word address to write to
 * \param[in] data pointer to tx buffer
 * \return status of the operation
 */
uint8_t aes132p_write_memory_physical(uint8_t count, uint16_t word_address, uint8_t *data)
{
	uint8_t aes132_lib_return;
	uint8_t writeEnable = AES132_SPI_ENABLE_WRITE;
	uint8_t preface[] =
		{AES132_SPI_WRITE, (uint8_t) (word_address >> 8), (uint8_t) (word_address & 0xFF)};

	// We don't need to enable write when writing to I/O address,
	// but an "if" condition would increase code space.
	spi_select_slave_phys();
	aes132_lib_return = spi_send_bytes(1, &writeEnable);
	spi_deselect_slave_phys();

	if (aes132_lib_return != AES132_FUNCTION_RETCODE_SUCCESS)
		return aes132_lib_return;

	// Here we could check the write-enable bit in the device status register,
	// but that would cost time and code space.
	spi_select_slave_phys();
	aes132_lib_return = spi_send_bytes(AES132_SPI_PREFACE_SIZE, preface);
	if (aes132_lib_return == AES132_FUNCTION_RETCODE_SUCCESS)
		aes132_lib_return = spi_send_bytes(count, data);

	spi_deselect_slave_phys();

	return aes132_lib_return;
}


/** \brief This function reads bytes from the device.
 * \param[in] size number of bytes to read
 * \param[in] word_address word address to read from
 * \param[out] data pointer to rx buffer
 * \return status of the operation
 */
uint8_t aes132p_read_memory_physical(uint8_t size, uint16_t word_address, uint8_t *data)
{
	uint8_t aes132_lib_return;
	uint8_t preface[] =
		{AES132_SPI_READ, (uint8_t) (word_address >> 8), (uint8_t) (word_address & 0xFF)};

	spi_select_slave_phys();
	aes132_lib_return = spi_send_bytes(AES132_SPI_PREFACE_SIZE, preface);
	if (aes132_lib_return == AES132_FUNCTION_RETCODE_SUCCESS)
		aes132_lib_return = spi_receive_bytes(size, data);

	spi_deselect_slave_phys();

	return aes132_lib_return;
}


/** \brief This function implements the interface for resynchronization
 * of communication, but cannot be implemented when using SPI.
 * \return always success
 */
uint8_t aes132p_resync_physical(void)
{
	return AES132_FUNCTION_RETCODE_SUCCESS;
}
