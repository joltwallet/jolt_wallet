/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_SYSCORE_FS_CONSOLE_H__
#define __JOLT_SYSCORE_FS_CONSOLE_H__

#define MAX_FILE_SIZE (1048576 - 0x2000)
#define SPIFFS_BASE_PATH "/spiffs"

uint16_t jolt_fs_get_all_elf_fns(char ***fns);
uint32_t jolt_fs_get_all_fns(char **fns, uint32_t fns_len,
        const char *ext, bool remove_ext);

void console_syscore_fs_register();
void filesystem_init() ;

#endif
