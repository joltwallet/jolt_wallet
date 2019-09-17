/**
 * @file cli_sub.h
 * @brief Interprets the Bluetooth and UART CLI.
 * @author Brian Pugh
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

#include "esp_console.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lvgl/lvgl.h"

typedef struct jolt_cli_sub_t {
    esp_console_cmd_t cmd;
    struct jolt_cli_sub_t *next;
} jolt_cli_sub_t;

/**
 * @brief Create a subconsole object that an application can register commands to
 * @return Subconsole
 */
jolt_cli_sub_t *jolt_cli_sub_init();

/**
 * @brief Register a command to the subconsole
 * @param[in,out] subconsole Subconsole object
 * @param[in], cmd Command to register. Does not need to persist.
 * @return 0 on success.
 */
int jolt_cli_sub_cmd_register( jolt_cli_sub_t *subconsole, esp_console_cmd_t *cmd );

/**
 * @brief execute the cmd.
 * @param[in] subconsole Subconsole object
 * @param argc
 * @return Command's return code. Returns -100 if command is not found.
 */
int jolt_cli_sub_cmd_run( jolt_cli_sub_t *subconsole, uint8_t argc, char **argv );

/**
 * @brief De allocate a subconsole.
 * @param[in] subconsole Subconsole object to deallocate.
 */
void jolt_cli_sub_cmd_free( jolt_cli_sub_t *subconsole );

#endif
