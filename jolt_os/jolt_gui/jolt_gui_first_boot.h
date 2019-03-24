/**
 * @file jolt_gui_first_boot.h
 * @brief Handle all GUI elements related to setting up the device on first run
 * @author Brian Pugh
 */

#ifndef __JOLT_GUI_FIRST_BOOT_H__
#define __JOLT_GUI_FIRST_BOOT_H__

#include "jolttypes.h"

/**
 * @brief Create the first first-boot screen
 *
 * This will generate subsequent screens via callbacks
 */
void jolt_gui_first_boot_create();

/**
 * @brief Save mnemonic to storage, using the same screens from first-boot
 * @param[in] mnemonic 256-bit binary respresentation of the mnemonic
 */
void jolt_gui_restore_sequence(const uint256_t mnemonic);

#endif
