/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_SYSCORE_FS_CONSOLE_H__
#define __JOLT_SYSCORE_FS_CONSOLE_H__

#define MAX_FILE_SIZE (1048576 - 0x2000)
#define SPIFFS_BASE_PATH "/spiffs"

uint32_t get_all_fns(char **fns, uint32_t fns_len, const char *ext, bool remove_ext);
char **malloc_char_array(int n);
void free_char_array(char **arr, int n);

void console_syscore_fs_register();
void filesystem_init() ;

#endif
