/*
 * spi_phys.c
 *
 * Created: 6/8/2015 1:42:41 PM
 *  Author: Cempaka
 */ 
#include "spi_phys.h"

//! [dev_inst]
struct spi_module spi_master_instance;
//! [dev_inst]
//! [slave_dev_inst]
struct spi_slave_inst slave[SPI_DEVICE_COUNT];
//! [slave_dev_inst]
//! [setup]

/** \brief This function assigns the chip select pin to be used for communication.
 *
 *         The same SPI peripheral is used (clock and data lines), but for the
 *         chip select, a different port and / or pin can be assigned.
 * @param index index into the port and pin arrays
 * @return error if index argument is out of bounds
 */
uint8_t spi_select_device(uint8_t index)
{
	if (index >= SPI_DEVICE_COUNT)
		return AES132_FUNCTION_RETCODE_DEVICE_SELECT_FAIL;
	else
	{
		spi_current_index = index;
		return SPI_FUNCTION_RETCODE_SUCCESS;
	}
}


/** This function selects the SPI slave. */
void spi_select_slave_phys(void)
{
	//! [select_slave]
		spi_select_slave(&spi_master_instance, &slave[spi_current_index], true);
	//! [select_slave]
}


/** This function deselects the SPI slave. */
void spi_deselect_slave_phys(void)
{
	//! [select_slave]
		spi_select_slave(&spi_master_instance, &slave[spi_current_index], false);
	//! [select_slave]
}


/** \brief This function initializes and enables the SPI peripheral. */
void spi_enable_phys(void)
{
	//! [config]
	struct spi_config config_spi_master;
	//! [config]
	//! [slave_config]
	struct spi_slave_inst_config slave_dev_config;
	//! [slave_config]
	/* Configure and initialize software device instance of peripheral slave */
	//! [slave_conf_defaults]
	spi_slave_inst_get_config_defaults(&slave_dev_config);
	//! [slave_conf_defaults]
	//! [ss_pin]
	slave_dev_config.ss_pin = spi_select[spi_current_index];
	//! [ss_pin]
	//! [slave_init]
	spi_attach_slave(&slave[spi_current_index], &slave_dev_config);
	//! [slave_init]
	/* Configure, initialize and enable SERCOM SPI module */
	//! [conf_defaults]
	spi_get_config_defaults(&config_spi_master);
	//! [conf_defaults]
	//! [set_baudrate]
	config_spi_master.mode_specific.master.baudrate = 4000000;
	//! [set_baudrate]
	//! [mux_setting]
	config_spi_master.mux_setting = EXT3_SPI_SERCOM_MUX_SETTING;
	//! [mux_setting]
	/* Configure pad 0 for data in */
	//! [di]
	config_spi_master.pinmux_pad0 = EXT3_SPI_SERCOM_PINMUX_PAD0;
	//! [di]
	/* Configure pad 1 as unused */
	//! [ss]
	config_spi_master.pinmux_pad1 = PINMUX_UNUSED;
	//! [ss]
	/* Configure pad 2 for data out */
	//! [do]
	config_spi_master.pinmux_pad2 = EXT3_SPI_SERCOM_PINMUX_PAD2;
	//! [do]
	/* Configure pad 3 for SCK */
	//! [sck]
	config_spi_master.pinmux_pad3 = EXT3_SPI_SERCOM_PINMUX_PAD3;
	//! [sck]
	//! [init]
	spi_init(&spi_master_instance, EXT3_SPI_MODULE, &config_spi_master);
	//! [init]

	//! [enable]
	spi_enable(&spi_master_instance);
	//! [enable]
}


/** \brief This function disables the SPI peripheral. */
void spi_disable_phys(void)
{
	// Disable SPI.
	spi_disable(&spi_master_instance);
}


/** \brief This function sends bytes to an SPI device.
 *
 * \param[in] count number of bytes to send
 * \param[in] data pointer to tx buffer
 * \return status of the operation
 */
uint8_t spi_send_bytes(uint8_t count, uint8_t *data)
{
	uint8_t ret_code = SPI_FUNCTION_RETCODE_SUCCESS;


	ret_code = spi_write_buffer_wait(&spi_master_instance, data, count);
		
	if (ret_code == STATUS_ERR_TIMEOUT)
	{
		return SPI_FUNCTION_RETCODE_TIMEOUT;
	}
	else if (ret_code != STATUS_OK)
	{
		return SPI_FUNCTION_RETCODE_COMM_FAIL;
	}
	else
	{
		return SPI_FUNCTION_RETCODE_SUCCESS;	
	}
}


/** \brief This function receives bytes from an SPI device.
 *
 * \param[in] count number of bytes to receive
 * \param[in] data pointer to rx buffer
 * \return status of the operation
 */
uint8_t spi_receive_bytes(uint8_t count, uint8_t *data)
{
	uint8_t ret_code = SPI_FUNCTION_RETCODE_SUCCESS;


	ret_code = spi_read_buffer_wait(&spi_master_instance, data, count,0xFF);
	
	if (ret_code == STATUS_ERR_TIMEOUT)
	{
		return SPI_FUNCTION_RETCODE_TIMEOUT;
	}
	else if (ret_code != STATUS_OK)
	{
		return SPI_FUNCTION_RETCODE_COMM_FAIL;
	}
	else
	{
		return SPI_FUNCTION_RETCODE_SUCCESS;
	}
}