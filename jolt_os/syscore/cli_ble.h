#ifndef JOLT_SYSCORE_CLI_BLE_H__
#define JOLT_SYSCORE_CLI_BLE_H__

#include "syscore/cli.h"

/**
 * @brief Setup the UART to be hooked up to the CLI engine.
 *     * configures UART communications.
 *     * linenoise for parsing commands.
 *     * UART listening task
 */
void jolt_cli_ble_init();

#endif
