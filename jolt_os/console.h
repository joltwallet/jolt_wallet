/**
 * @file console.h
 * @brief Interprets the Bluetooth and UART CLI.
 * @author Brian Pugh
 * @bugs 
 *     * Rework the console blocking/nonblocking/channel-ownership system.
 *     * Stronger subconsole system + documentation.
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

#include "esp_console.h"
#include "lvgl/lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * @brief object that represents a command job to process in the BG task.
 */
typedef struct jolt_cmd_t {
    SemaphoreHandle_t complete; /**< Indicates Job is done */ // todo: possibly remove this for a more elegant way of "taking over" BLE/UART communication.
    int return_value;           /**< */ // todo: see above
    char *data;                 /**< User command string */
    FILE *fd_in;                /**< FD the command was inputted on*/
    FILE *fd_out;               /**< FD to print response on */
    FILE *fd_err;               /**< FD to print errors to*/
} jolt_cmd_t;

/**
 * @brief Initialize the console module.
 *
 * Initializes the following:
 *     * configures UART communications.
 *     * linenoise for parsing commands.
 *     * registers system commands.
 *
 * Restarts device on failure.
 */
void console_init();

/**
 * @brief Creates the FreeRTOS task that handles UART inputs.
 * @return task handle
 */
TaskHandle_t *console_start();

/**
 * @brief Packages up parameters into a background job to be processed
 * @param[in] line NULL-terminated command string.
 * @param[in] in in file stream
 * @param[in] out out file stream
 * @param[in] err err file stream
 * @param[in] block Function blocks if True, otherwise non-blocking.
 * @return Returns 0 on success.
 */
int jolt_cmd_process(char *line, FILE *in, FILE *out, FILE *err, bool block);

/**
 * @brief Deallocates cmd job resources.
 *
 * NOT used to cancel a job. A job currently cannot be aborted from queue.
 *
 * @param[in,out] cmd cmd job to deallocate
 */
void jolt_cmd_del(jolt_cmd_t *cmd);


/*******************************************************
 * SUBCONSOLE - console functionality for applications *
 *******************************************************/

typedef struct subconsole_t {
    esp_console_cmd_t cmd;
    struct subconsole_t *next;
} subconsole_t;

/**
 * @brief Create a subconsole object that an application can register commands to 
 * @return Subconsole
 */
subconsole_t *subconsole_cmd_init();

/**
 * @brief Register a command to the subconsole
 * @param[in,out] subconsole Subconsole object
 * @param[in], cmd Command to register. Does not need to persist.
 * @return 0 on success. 
 */
int subconsole_cmd_register(subconsole_t *subconsole, esp_console_cmd_t *cmd);

/**
 * @brief execute the cmd. 
 * @param[in] subconsole Subconsole object
 * @param argc
 * @return Command's return code. Returns -100 if command is not found.
 */
int subconsole_cmd_run(subconsole_t *subconsole, uint8_t argc, char **argv);

/**
 * @brief De allocate a subconsole.
 * @param[in] subconsole Subconsole object to deallocate.
 */
void subconsole_cmd_free(subconsole_t *subconsole);

#endif
