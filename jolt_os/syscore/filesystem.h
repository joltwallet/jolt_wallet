/**
 * @file filesystem.h
 * @brief Primary filesystem helpers
 * @author Brian Pugh
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_SYSCORE_FS_CONSOLE_H__
#define __JOLT_SYSCORE_FS_CONSOLE_H__

#include "stdbool.h"

/**
 * @brief Minimum amount of free space allowed in bytes.
 *
 * Don't let the filesystem fill up 100% 
 */
#define FS_MIN_FREE 0x2000

/**
 * @brief Maximum file size allowed in bytes
 */
#define MAX_FILE_SIZE 524288

/**
 * @brief Mounting point for spiffs filesystem
 */
#define SPIFFS_BASE_PATH "/spiffs"

/**
 * @brief Allocates and returns the names of all files ending with ".jelf" 
 * @param[out] array of NULL-terminated strings.
 * @return Number of NULL-terminated strings returned
 */
uint16_t jolt_fs_get_all_jelf_fns(char ***fns);

/**
 * @brief Get all filenames
 *
 * Returns upto fns_len fns with extension ext and the number of files.
 * If fns is NULL, just return the file count.
 * If ext is NULL, return all files
 * Uses malloc to reserve space for fns
 *
 * @param[out] fns String array to populate. If NULL, only the file count will be returned
 * @param[in] Length of fns string pointer array. fns_len Ignored if fns is NULL
 * @param[in] ext NULL-terminated string of extension to filter by (e.x. ".jelf"). NULL for all files.
 * @param[in] remove_ext return files without ext. Saves memory.
 *
 * @return Number of files 
 */
uint32_t jolt_fs_get_all_fns(char **fns, uint32_t fns_len,
        const char *ext, bool remove_ext);

/**
 * @brief Starts up the SPIFFS Filesystem
 */
void jolt_fs_init() ;

/**
 * @brief Get the number of available unused bytes 
 *
 * Actually returns FS_MIN_FREE less bytes.
 *
 * @return Number of free bytes in the filesystem 
 */
uint32_t jolt_fs_free();

/**
 * @brief Get the storage size of a file 
 * @param[in] full filename path; i.e. '/spiffs/test.txt 
 * @return size of file in bytes. Returns -1 if the file is not found. 
 */
size_t jolt_fs_size(const char *fname);

/**
 * @brief Checks if file exists. 
 * @param[in] full filename path; i.e. '/spiffs/test.txt 
 * @return 0 if file exists, 1 if file does not exist, -1 if filesystem is not mounted.
 */
bool jolt_fs_exists(const char *fname);

#endif
