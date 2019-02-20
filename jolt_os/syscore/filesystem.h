/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_SYSCORE_FS_CONSOLE_H__
#define __JOLT_SYSCORE_FS_CONSOLE_H__

/* Don't let the filesystem fill up 100% */
#define FS_MIN_FREE 0x2000
#define MAX_FILE_SIZE (1048576 - FS_MIN_FREE)
#define SPIFFS_BASE_PATH "/spiffs"

uint16_t jolt_fs_get_all_jelf_fns(char ***fns);
uint32_t jolt_fs_get_all_fns(char **fns, uint32_t fns_len,
        const char *ext, bool remove_ext);

void jolt_fs_init() ;

uint32_t jolt_fs_free();
size_t jolt_fs_size(char *fname);
int jolt_fs_exists(char *fname);

#endif
