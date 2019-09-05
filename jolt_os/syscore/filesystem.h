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
#include <stdarg.h>
#include "sdkconfig.h"
#include "esp_err.h"

/**
 * @brief Mounting point for filesystem
 * 
 * Must NOT end in "/"
 */
#define JOLT_FS_MOUNTPT "/store"

#define JOLT_FS_PARTITION "storage"

/**
 * @brief Minimum amount of free space allowed in bytes.
 *
 * Don't let the filesystem fill up 100% 
 */
#define JOLT_FS_MIN_FREE 0x2000

/**
 * @brief Maximum file size allowed in bytes
 * NOTE: Currently not used.
 */
#define JOLT_FS_MAX_FILE_SIZE 524288

/**
 * @brief inclusive max strlen of a filename.
 *
 * DOES NOT account for null terminator. The base_path does not contribute to overall length.
 *
 * e.g. "/store/foo.bar" is 7 long and is <= JOLT_FS_MAX_FILENAME_LEN, so its ok.
 *
 */
#if CONFIG_JOLT_FS_SPIFFS
    #define JOLT_FS_MAX_FILENAME_LEN CONFIG_SPIFFS_OBJ_NAME_LEN-2
#elif CONFIG_JOLT_FS_LITTLEFS 
    #define JOLT_FS_MAX_FILENAME_LEN CONFIG_LITTLEFS_OBJ_NAME_LEN-2
#elif CONFIG_JOLT_FS_FAT
    #define JOLT_FS_MAX_FILENAME_LEN 126 //TODO refine
#endif

/**
 * @brief inclusive max buflen of a filename.
 * 
 * Takes into account the NULL terminator.
 */
#define JOLT_FS_MAX_FILENAME_BUF_LEN JOLT_FS_MAX_FILENAME_LEN+1

/**
 * @brief buffer length to hold the longest null-terminated full path.
 */
#define JOLT_FS_MAX_ABS_PATH_BUF_LEN sizeof(JOLT_FS_MOUNTPT) + JOLT_FS_MAX_FILENAME_BUF_LEN

#define JOLT_FS_TMP_FN (JOLT_FS_MOUNTPT "/tmp")

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
 * @brief Starts up the Filesystem
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
 * @param[in] full filename path; i.e. '/store/test.txt'
 * @return size of file in bytes. Returns -1 if the file is not found. 
 */
size_t jolt_fs_size(const char *fname);

/**
 * @brief Checks if file exists. 
 * @param[in] full filename path; i.e. '/store/test.txt' or just the filename 'test.txt'.
 * @return 0 if file exists, 1 if file does not exist, -1 if filesystem is not mounted.
 */
bool jolt_fs_exists(const char *fname);

/**
 * @brief Parse a file path.
 *
 * Populates with filesystem mount point if it's not already.
 *
 * ex.
 *     char *path1 = jolt_fs_parse("Jolt", "jelf");
 *     // "/store/Jolt.jelf"
 *     char *path2 = "jolt_fs_parse("Jolt", NULL);
 *     // "/store/Jolt"
 *     char *path3 = "jolt_fs_parse("Jolt", ".jelf");
 *     // "/store/Jolt.jelf"
 *     char *path4 = "jolt_fs_parse("/store/Jolt", ".jelf");
 *     // "/store/Jolt.jelf"

 *
 * @param[in] fn Filename. 
 * @param[in] ext extension with or without the "." Can be NULL for no ext.
 * @return Allocated string representing the filename. NULL on failure. Must be freed.
 */
char * jolt_fs_parse(const char *fn, const char *ext);

/** 
 * @brief Allocates the maximum allowed file name buffer size.
 *
 * Populates with filesystem mount point if it's not already.
 *
 * NOTE: only free fullpath, NOT fn.
 *
 * @param[out] fullpath Full path "/store/..."
 * @param[out] fn [optional] Pointer to the filename portion of the buffer "...".
 */
void jolt_fs_parse_buf(char **fullpath, char **fn);

/**
 * @brief Strip the extension from the file inplace
 * @param[in,out] fname File path to strip.
 */
void jolt_fs_strip_ext(char *fname);

/**
 * @brief Move a file, deleting the old destination if it existed.
 *
 * Filenames MUST be fullpaths ("/store/foo.bar")
 *
 * @param[in] src
 * @param[in] dst
 */
esp_err_t jolt_fs_mv(const char *src, const char *dst);

/**
 * @brief Erase and format the filesystem
 * @return ESP_OK on success.
 */
esp_err_t jolt_fs_format();

/**
 * Get information for jolt's filesystem
 *
 * @param partition_label           Optional, label of the partition to get info for.
 * @param[out] total_bytes          Size of the file system
 * @param[out] used_bytes           Current used bytes in the file system
 *
 * @return  
 *          - ESP_OK                  if success
 *          - ESP_ERR_INVALID_STATE   if not mounted
 */
esp_err_t jolt_fs_info(size_t *total_bytes, size_t *used_bytes);

/**
 * @brief Get the filesystem type.
 *
 * @return NULL-terminated string literal.
 */
char *jolt_fs_type();

#endif
