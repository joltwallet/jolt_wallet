/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_SYSCORE_LAUNCHER_H__
#define __JOLT_SYSCORE_LAUNCHER_H__

int launch_file(const char *fn_basename, const char *func, int app_argc, char** app_argv);
void console_syscore_launcher_register();

#endif
