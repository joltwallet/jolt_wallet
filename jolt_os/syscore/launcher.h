/**
 * @file launcher.h
 * @brief Components interfacing the GUI and filesystem to the JELF Loader.
 * @author Brian Pugh
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_SYSCORE_LAUNCHER_H__
#define __JOLT_SYSCORE_LAUNCHER_H__

#include "stdbool.h"

/**
 * @brief Runs a Jolt Application file.
 *
 *
 * If the app was the last app launched (or currently launched app), it will
 * use the last cached instance (unless vault has been invalidated and 
 * matches derivation path)
 *
 * @param[in] fn_basename app specified without ".elf" suffix. i.e. "app"
 * @param[in] app_argc argument count to pass in to application.
 * @param[in] app_argv variable arguments
 * @param[in] passphrase string to use during BIP32 derivation
 * @return application's return code.
 */
int launch_file(const char *fn_basename, int app_argc, char** app_argv, const char *passphrase);

/**
 * @brief Checks if the user is in an application
 * @return Returns True if User is in an app (screen is non NULL) 
 */
bool launch_in_app();

/**
 * @brief Get the name of the currently cached app.
 * @return name of currently chached app (without ".elf" suffix).
 */
char *launch_get_name();

/**
 * @brief Increment the application reference counter.
 *
 * An application can only be unloaded when the reference counter is 0.
 */
void launch_inc_ref_ctr();

/**
 * @brief Decrement the application reference counter.
 */
void launch_dec_ref_ctr();

#endif
