/*
 * i2c_phys.c
 *
 * Created: 6/8/2015 1:43:37 PM
 *  Author: Cempaka
 */ 

#include "i2c_phys.h"
#include "driver/i2c.h" /* esp-idf i2c library */
#include "esp_log.h"
#include "esp_err.h"
#include "../../globals.h"

/** \brief I2C address used at AES132 library startup. */
#define AES132_I2C_DEFAULT_ADDRESS   ((uint8_t) 0xA0)

static const char TAG[] = "i2c_phys";

//! I2C address currently in use.
static uint8_t i2c_address_current = AES132_I2C_DEFAULT_ADDRESS;

/* Helper error translating/logging functions */
static uint8_t i2c_master_cmd_begin_s(i2c_port_t i2c_num,
        i2c_cmd_handle_t cmd, TickType_t ticks_to_wait)
{
    uint8_t res = I2C_FUNCTION_RETCODE_COMM_FAIL;
    ESP_LOGD(TAG, "Taking display mutex");
    xSemaphoreTake(disp_mutex, portMAX_DELAY);
    switch( i2c_master_cmd_begin(i2c_num, cmd, ticks_to_wait) ) {
        case ESP_OK:
            ESP_LOGD(TAG, "I2C buffer send success.");
            res = I2C_FUNCTION_RETCODE_SUCCESS;
            break;
        case ESP_ERR_INVALID_ARG:
            ESP_LOGW(TAG, "i2c_master_cmd_begin parameter error.");
            break;
        case ESP_FAIL:
            ESP_LOGD(TAG, "Slave doesn’t ACK the transfer. "
                    "ataes132a is probably busy carrying out a command.");
            res = I2C_FUNCTION_RETCODE_NACK;
            break;
        case ESP_ERR_INVALID_STATE:
            ESP_LOGE(TAG, "I2C driver not installed or not in master mode.");
            break;
        case ESP_ERR_TIMEOUT:
            ESP_LOGW(TAG, "Operation timeout because the bus is busy.");
            res = I2C_FUNCTION_RETCODE_TIMEOUT;
            break;
    }
    ESP_LOGD(TAG, "Giving display mutex");
    xSemaphoreGive(disp_mutex);
    return res;
}

/** \brief This function selects a I2C AES132 device.
 *
 * @param[in] device_id I2C address
 * @return always success
 */
uint8_t i2c_select_device_phys(uint8_t device_id)
{		
	i2c_address_current = device_id & ~1; // Mask off r/w bit
	return AES132_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function initializes and enables the I2C peripheral.
 * */
void i2c_enable_phys(void)
{
    // Done; need to debug

    esp_err_t err;
    // Configure I2C Driver
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;

    ESP_LOGI(TAG, "sda_io_num %d", CONFIG_JOLT_I2C_PIN_SDA);
    conf.sda_io_num = CONFIG_JOLT_I2C_PIN_SDA;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;

    ESP_LOGI(TAG, "scl_io_num %d", CONFIG_JOLT_I2C_PIN_SCL);
    conf.scl_io_num = CONFIG_JOLT_I2C_PIN_SCL;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;

    ESP_LOGI(TAG, "clk_speed %d", CONFIG_JOLT_I2C_MASTER_FREQ_HZ);
    conf.master.clk_speed = CONFIG_JOLT_I2C_MASTER_FREQ_HZ;

    ESP_LOGI(TAG, "i2c_param_config %d", conf.mode);
    err = i2c_param_config(CONFIG_JOLT_I2C_MASTER_NUM, &conf);

    // Install i2c driver
    ESP_LOGI(TAG, "i2c_driver_install %d", CONFIG_JOLT_I2C_MASTER_NUM);
    err = i2c_driver_install(CONFIG_JOLT_I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    if(ESP_FAIL == err) {
        ESP_LOGW(TAG, "ATAES132A failed to install I2C driver. "
                "I2C driver may already be installed");
    }
}


/** \brief This function disables the I2C peripheral. */
void i2c_disable_phys(void)
{
    i2c_driver_delete(CONFIG_JOLT_I2C_MASTER_NUM);
}

/** \brief This function sends bytes to an I2C device.
 * \param[in] count number of bytes to send
 * \param[in] data pointer to tx buffer
 * \return status of the operation
 */
uint8_t i2c_send_bytes(uint8_t count, uint8_t *data)
{
    /* Data contains complete message (except device address). This means:
     * 0-1) Memory address
     * 2) Message Count
     * 3-N) Data
     * N+1, N+2) CRC */
    for(uint8_t i=0; i<count; i++) {
        ESP_LOGD(TAG, "Send byte %d: %.2X", i, data[i]);
    }

    uint8_t res = I2C_FUNCTION_RETCODE_COMM_FAIL;
    i2c_cmd_handle_t cmd;
    if( NULL == (cmd = i2c_cmd_link_create()) ) {
        ESP_LOGE(TAG, "Failed to create i2c_cmd_link");
        return I2C_FUNCTION_RETCODE_COMM_FAIL;
    }
    if( ESP_OK != i2c_master_start(cmd) ) {
        ESP_LOGE(TAG, "i2c_master_start parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_write_byte(cmd, i2c_address_current | I2C_MASTER_WRITE, ACK_CHECK_EN) ) {
        ESP_LOGE(TAG, "i2c_master_write_byte slave address parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_write(cmd, data, count, ACK_CHECK_EN) ) {
        ESP_LOGE(TAG, "i2c_master_write payload data parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_stop(cmd) ) {
        ESP_LOGE(TAG, "i2c_master_stop parameter error");
        goto failure;
    }
    res = i2c_master_cmd_begin_s(CONFIG_JOLT_I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
failure:
    i2c_cmd_link_delete(cmd);
    return res;
}

/** \brief This function receives one byte from an I2C device.
 *
 * \param[out] data pointer to received byte
 * \return status of the operation
 */
uint8_t i2c_receive_byte(uint8_t *data, uint8_t *address)
{
    return i2c_receive_bytes(1, data, address);
}

/** \brief This function receives bytes from an I2C device
 *         and sends a Stop.
 *
 * \param[in] count number of bytes to receive
 * \param[out] data pointer to rx buffer
 * \return status of the operation
 */
uint8_t i2c_receive_bytes(uint8_t count, uint8_t *data, uint8_t *address)
{
	// Random read:
	// Start, I2C address with write bit, word address, Start, I2C address with read bit

    ESP_LOGD(TAG, "Performing read from memory address %.2X %.2X",
            address[0], address[1]);
    uint8_t res = I2C_FUNCTION_RETCODE_COMM_FAIL;
    i2c_cmd_handle_t cmd;
    if( NULL == (cmd = i2c_cmd_link_create()) ) {
        ESP_LOGE(TAG, "Failed to create i2c_cmd_link");
        return I2C_FUNCTION_RETCODE_COMM_FAIL;
    }
    if( ESP_OK != i2c_master_start(cmd) ) {
        ESP_LOGE(TAG, "i2c_master_start parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_write_byte(cmd, i2c_address_current | I2C_MASTER_WRITE, ACK_CHECK_EN) ) {
        ESP_LOGE(TAG, "i2c_master_write_byte slave address parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_write(cmd, address, 2, ACK_CHECK_EN) ) {
        ESP_LOGE(TAG, "i2c_master_write payload data parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_start(cmd) ) {
        ESP_LOGE(TAG, "i2c_master_start parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_write_byte(cmd, i2c_address_current | I2C_MASTER_READ, ACK_CHECK_EN) ) {
        ESP_LOGE(TAG, "i2c_master_write_byte slave address parameter error");
        goto failure;
    }
    ESP_LOGD(TAG, "Going to read in %d bytes into data", count);
    if( ESP_OK != i2c_master_read(cmd, data, count, I2C_MASTER_LAST_NACK) ) {
        ESP_LOGE(TAG, "i2c_master_read parameter error");
        goto failure;
    }
    if( ESP_OK != i2c_master_stop(cmd) ) {
        ESP_LOGE(TAG, "i2c_master_stop parameter error");
        goto failure;
    }
    res = i2c_master_cmd_begin_s(CONFIG_JOLT_I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    for(uint8_t i=0; i<count; i++) {
        ESP_LOGD(TAG, "Received byte %d: %.2X", i, data[i]);
    }
failure:
    i2c_cmd_link_delete(cmd);
    return res;
}
