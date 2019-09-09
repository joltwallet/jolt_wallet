/**
 * @file jolt_cmds.h
 * @brief CLI programs
 * @author Brian Pugh
 * @bugs pmtop deadlocks system; needs to be fixed upstream in ESP-IDF.
 */

#ifndef JOLT_CMDS_H__
#define JOLT_CMDS_H__

#include "sdkconfig.h"

/**
 * @brief Report versioning and other system data
 *
 * No args:
 *     Print general system info
 * 1 args:
 *     wifi - print wifi info
 *     filesystem - print filesystem info
 */
int jolt_cmd_about(int argc, char** argv);

/**
 * @brief Sets app public key. WILL ERASE ALL DATA. Does not require PIN.
 *
 * If no argument is provided, will return the current app_key.
 *
 * Takes 1 argument: the new 256-bit hexidecimal app_key (64-characters long).
 */
int jolt_cmd_app_key(int argc, char** argv);

/**
 * @brief Print BLE GAP White List
 *
 * Takes no arguments
 */
int jolt_cmd_bt_whitelist(int argc, char** argv);

/**
 * @brief Print the contents of a file
 *
 * Takes 1 argument: file to print
 */
int jolt_cmd_cat(int argc, char** argv);

/**
 * @brief Send specified file from Jolt over UART ymodem
 *
 * Takes 1 argument: filename to transfer
 */
int jolt_cmd_download(int argc, char** argv);

/**
 * @brief Get the total size of heap memory available
 *
 * Takes no arguments
 */
int jolt_cmd_free(int argc, char** argv);

/**
 * @brief Get the number of lv_objs
 *
 * Takes no arguments
 */
int jolt_cmd_lv_obj(int argc, char** argv);

/**
 * @brief Update jolt_cast URI.
 *
 * Takes 1 argument: the new URI
 */
int jolt_cmd_jolt_cast_update(int argc, char** argv);

/**
 * @brief List all files in filesystem
 *
 * Takes no arguments
 *
 * keys:
 *     n - [int] Number of files
 *     free - [int] Space free (bytes)
 *     total - [int[ Total disk size (bytes)
 *     files - [list]
 *         name - [str] Name of file
 *         size - [int] Size of file (bytes)
 */
int jolt_cmd_ls(int argc, char** argv);

/**
 * @brief Restore mnemonic seed
 *
 * Takes no arguments
 */
int jolt_cmd_mnemonic_restore(int argc, char** argv);

/**
 * @brief rename file (src, dst)
 * 
 * Takes 2 arguments (src, dst)
 */
int jolt_cmd_mv(int argc, char** argv);

/**
 * @brief print power management statistics
 *
 * Takes no arguments
 */
int jolt_cmd_pmtop(int argc, char** argv);

/**
 * @brief Reboot device.
 *
 * Takes no arguments
 */
int jolt_cmd_reboot(int argc, char** argv);

/**
 * @brief remove file from filesystem
 *
 * Takes 1 argument: File to remove.
 */
int jolt_cmd_rm(int argc, char** argv);

/**
 * @brief generate random data
 *
 * Takes 1 mandatory argument: Number of bytes of random data to generate. -1 for infinite.
 *
 * Takes 1 optional flag: "--hex" to output in hexidecimal
 */
int jolt_cmd_rng(int argc, char** argv);

/**
 * @brief Memory usage of all running tasks.
 *
 * Takes no arguments
 */
int jolt_cmd_task_status(int argc, char** argv);

/**
 * @brief CPU-Usage of Tasks.
 *
 * Takes no arguments
 */
int jolt_cmd_top(int argc, char** argv);

/**
 * @brief Enters file UART ymodem upload mode
 *
 * Takes no arguments
 */
int jolt_cmd_upload(int argc, char** argv);

#if JOLT_GUI_TEST_MENU
/**
 * Prompt user for PIN to unlock device.
 *
 * Takes no arguments.
 */
int jolt_cmd_unlock(int argc, char** argv);
#endif

/**
 * @brief Update JoltOS
 *
 * Takes no arguments
 */
int jolt_cmd_upload_firmware(int argc, char** argv);

/**
 * @brief Update WiFi SSID and Pass.
 *
 * Takes 1 or 2 arguments:
 *  1 argument: SSID, assumes no password
 *  2 arguments: SSID and password
 */
int jolt_cmd_wifi_update(int argc, char** argv);
#endif
