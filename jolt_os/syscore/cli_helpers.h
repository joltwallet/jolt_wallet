/**
 * @file cli_helpers.h
 * @brief Common CLI helpers
 * @author Brian Pugh
 */

#ifndef JOLT_SYSCORE_CLI_HELPERS_H__
#define JOLT_SYSCORE_CLI_HELPERS_H__

#include "stdint.h"
#include "stdbool.h"
#include "jolt_helpers.h"

/**
 * @brief Check to see if the provided argument count is within specified range.
 * 
 * Prints error to STDOUT if not in range.
 *
 * @param[in] argc Argument count
 * @param[in] min Minimum argument count
 * @param[in] max Maximum argument count
 * @return True if argc is between [min, max] inclusive
 */
bool console_check_range_argc(uint8_t argc, uint8_t min, uint8_t max);

/**
 * @brief Check to see if the provided argument count is expected amount.
 *
 * Prints error to STDOUT if not expected amount.
 *
 * @param[in] argc Argument count
 * @param[in] max expected
 * @return True if argc is expected value
 */
bool console_check_equal_argc(uint8_t argc, uint8_t expected);

#endif
