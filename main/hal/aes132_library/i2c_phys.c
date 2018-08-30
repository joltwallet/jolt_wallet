/*
 * i2c_phys.c
 *
 * Created: 6/8/2015 1:43:37 PM
 *  Author: Cempaka
 */ 

#include "i2c_phys.h"
#include "i2c_master.h"

/** \brief I2C address used at AES132 library startup. */
#define AES132_I2C_DEFAULT_ADDRESS   ((uint8_t) 0xA0)

// File scope globals
// I2C software module.
struct i2c_master_module i2c_master_instance;

//! I2C address currently in use.
static uint8_t i2c_address_current = AES132_I2C_DEFAULT_ADDRESS;



/** \brief This function selects a I2C AES132 device.
 *
 * @param[in] device_id I2C address
 * @return always success
 */
uint8_t i2c_select_device_phys(uint8_t device_id)
{		
	i2c_address_current = device_id & ~1;
	return AES132_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function initializes and enables the I2C peripheral.
 * */
void i2c_enable_phys(void)
{
	/* Initialize config structure and software module. */
	//! [init_conf]
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);
	//! [init_conf]

	config_i2c_master.baud_rate = 400;

	/* Change buffer timeout to something longer. */
	//! [conf_change]
	config_i2c_master.buffer_timeout = 10000;
	//! [conf_change]

	/* Initialize and enable device with config. */
	//! [init_module]
	i2c_master_init(&i2c_master_instance, SERCOM2, &config_i2c_master);
	//! [init_module]

	//! [enable_module]
	i2c_master_enable(&i2c_master_instance);
	//! [enable_module]
}


/** \brief This function disables the I2C peripheral. */
void i2c_disable_phys(void)
{
	//! [enable_module]
	i2c_master_disable(&i2c_master_instance);
	//! [enable_module]
}


/** \brief This function creates a Start condition (SDA low, then SCL low).
 * \return status of the operation
 * */
uint8_t i2c_send_start(void)
{
	// Do nothing
	return I2C_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function creates a Stop condition (SCL high, then SDA high).
 * \return status of the operation
 * */
uint8_t i2c_send_stop(void)
{
	SercomI2cm *const i2c_module = &(i2c_master_instance.hw->I2CM);

	/* Send stop command unless arbitration is lost. */
	_i2c_master_wait_for_sync(&i2c_master_instance);
	i2c_module->CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(3);
	return I2C_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function sends bytes to an I2C device.
 * \param[in] count number of bytes to send
 * \param[in] data pointer to tx buffer
 * \return status of the operation
 */
uint8_t i2c_send_bytes(uint8_t count, uint8_t *data)
{
	enum status_code statusCode = STATUS_OK;
	struct i2c_master_packet packet = {
		.address     = i2c_address_current>>1,
		.data_length = count,
		.data        = data,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};

	//	statusCode = i2c_master_write_packet_wait(&i2c_master_instance, &packet);
	statusCode = i2c_master_write_packet_wait_no_stop(&i2c_master_instance, &packet);
	if (statusCode != STATUS_OK) return I2C_FUNCTION_RETCODE_COMM_FAIL;

	// Send the stop
	//statusCode = i2c_send_stop();
	//if (statusCode != STATUS_OK) return I2C_FUNCTION_RETCODE_COMM_FAIL;

	return I2C_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function receives one byte from an I2C device.
 *
 * \param[out] data pointer to received byte
 * \return status of the operation
 */
uint8_t i2c_receive_byte(uint8_t *data)
{

	return i2c_receive_bytes(1, data);
}


/** \brief This function receives bytes from an I2C device
 *         and sends a Stop.
 *
 * \param[in] count number of bytes to receive
 * \param[out] data pointer to rx buffer
 * \return status of the operation
 */
uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data)
{
	enum status_code statusCode = I2C_FUNCTION_RETCODE_SUCCESS;
	struct i2c_master_packet packet = {
		.address     = i2c_address_current>>1,
		.data_length = count,
		.data        = data,
		.ten_bit_address = false,
		.high_speed      = false,
		.hs_master_code  = 0x0,
	};

	statusCode = i2c_master_read_packet_wait(&i2c_master_instance, &packet);
	if (statusCode != STATUS_OK)
	{
		return I2C_FUNCTION_RETCODE_COMM_FAIL;
	}
	
	statusCode = i2c_send_stop();
	if (statusCode != I2C_FUNCTION_RETCODE_SUCCESS) return I2C_FUNCTION_RETCODE_COMM_FAIL;
	
	return I2C_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function creates a Start condition and sends the I2C address.
 * \param[in] read I2C_READ for reading, I2C_WRITE for writing
 * \return status of the operation
 */
uint8_t i2c_send_slave_address(uint8_t read)
{
	// Do nothing
	return AES132_FUNCTION_RETCODE_SUCCESS;
}
