/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_SYSCORE_LAUNCHER_H__
#define __JOLT_SYSCORE_LAUNCHER_H__

#include "stdbool.h"

/* Launches app specified without ".elf" suffix. i.e. "app"
 * Launches the app's function with same name as func.
 *
 * If the app was the last app launched (or currently launched app), it will
 * use the last cached instance (unless vault has been invalidated and 
 * matches derivation path)
 */
int launch_file(const char *fn_basename, int app_argc, char** app_argv);

/* Returns True if User is in an app (screen is non NULL) */
bool launch_in_app();

#endif
