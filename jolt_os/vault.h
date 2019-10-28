/**
 * @file vault.h
 * @brief Primary master mnemonic handling functions
 * @author Brian Pugh
 *
 * @bugs
 *     * Stack overflow on VaultWatchDog after a long time idle, possible memory leak.
 */

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_VAULT_H__
#define __JOLT_VAULT_H__

#include "bipmnemonic.h"
#include "jolttypes.h"
#include "lvgl/lvgl.h"
#include "nvs.h"

/**
 * @brief Maximum BIP32 Passphrase length (not including NULL character)
 */
#define CONFIG_JOLT_VAULT_PASSPHRASE_MAX_LEN 127
/**
 * @brief Maximum BIP32 Key length (not including NULL character)
 */
#define CONFIG_JOLT_VAULT_BIP32_KEY_MAX_LEN 31

/**
 * @brief Contains secret information available to an app.
 *
 * There is only a single instance of a vault in Jolt. The vault is to store
 * anything that if modified could perform something malicious.
 * A Vault only holds a single derivation node (along with the derivation path
 * describing it) as a security precaution. In the event an application is
 * exploited, damage is physically restricted to that single derivation path.
 *
 */
typedef struct vault_t {
    hd_node_t node;     /**< BIP derivation node to derive private keys */
    uint32_t purpose;   /**< first element of derivation path. Typically 44' */
    uint32_t coin_type; /**< The application's coin's path */
    char bip32_key[CONFIG_JOLT_VAULT_BIP32_KEY_MAX_LEN +
                   1]; /**< derivation key string. Typically "bitcoin_seed" or "ed25519 seed" */
    char passphrase[CONFIG_JOLT_VAULT_PASSPHRASE_MAX_LEN + 1]; /** The optional 25th mnemonic word */
    bool valid; /* True if node contains good data. False if node has been wiped */
} vault_t;

/**
 * @brief callback to perform after user attempts to enter a PIN
 * @param[in,out] param Arbitrary user data to pass to the callback
 */
typedef void ( *vault_cb_t )( void *param );

/**
 * @brief Take the vault's recursive semaphore
 *
 * Blocks until the semaphore is obtained.
 */
void vault_sem_take();

/**
 * @brief Releases the vault's recursive semaphore
 */
void vault_sem_give();

/**
 * @brief Allocates and initializes the Vault module.
 *
 * Allocates the Vault structure.
 * Initializes the Vault recursive semaphore.
 * Creates the VaultWatchDog FreeRTOS Task.
 *
 * @return Returns true on success. Returns false if no stored mnemonic data is found.
 */
bool vault_setup();

/**
 * @brief Clears the Vault struct of secrets.
 * Does NOT clear the following so that the node can be easily restored on demand:
 *      * purpose
 *      * coin_type
 *      * bip32_key
 */
void vault_clear();

/**
 * @brief Sets the vault (non-blocking).
 *
 * Internally will prompt the user for PIN, if necessary.
 *
 * @param[in] purpose Coin purpose; typically 44'
 * @param[in] coin_type Coin type
 * @param[in] bip32_key BIP32 Key; typically "bitcoin_seed"
 * @param[in] passphrase optional 25th mnemonic word. Empty if NULL.
 * @param[in] failure_cb Callback on PIN failure.
 * @param[in] success_cb Callback on PIN success
 * @param[in] param Arbitrary data to pass on to callback
 *
 * @return ESP_OK on success
 */
esp_err_t vault_set( uint32_t purpose, uint32_t coin_type, const char *bip32_key, const char *passphrase,
                     vault_cb_t failure_cb, vault_cb_t success_cb, void *param );

/**
 * @brief refreshes the node in the Vault. Will prompt for PIN if necessary.
 *
 * Internally kicks the VaultWatchDog. If the vault is no longer valid, the
 * Node wil be rederived based on the current Vault parameters.
 *
 * @param[in] failure_cb Callback on PIN failure.
 * @param[in] success_cb Callback on PIN success or valid vault.
 * @param[in] param Arbitrary data to pass on to callback
 */
void vault_refresh( vault_cb_t failure_cb, vault_cb_t success_cb, void *param );

/**
 * @brief reset the VaultWatchDog timer if vault is valid.
 * @return true if vault is valid, false otherwise
 */
bool vault_kick();

/**
 * @brief Get the coin_type
 * @return coin_type
 */
uint32_t vault_get_coin_type();

/**
 * @brief Get the purpose
 * @return purpose
 */
uint32_t vault_get_purpose();

/**
 * @brief Get the bip32_key
 *
 * bip32_key is a null-terminated string.
 *
 * @return bip32_key
 */
char *vault_get_bip32_key();

/**
 * @brief Get the derivation node
 *
 * NEVER make a local copy of the node.
 *
 * The general work flow for an application to use the secret is as follows:
 * 1. Refresh the vault (@see vault_refresh); this will ensure that the node has good data.
 * 2. Perform cryptographic operations. This is to be done immediately after
 *    refreshing the vault. It is assumed that the cryptographic operation
 *    takes much less than the VaultWatchDog timer.
 *
 * @return node
 */
hd_node_t *vault_get_node();

/**
 * @brief Checks if the vault contains valid secrets
 * @return True if vault is valid, false otherwise.
 */
bool vault_get_valid();

/**
 * @brief Convert a NULL-terminated len-2 derivation path string to a purpose and coin_type.
 * @param[in] str Input path string like "44'/165'"
 * @param[out] purpose
 * @param[out] coin_type
 * @return 0 on success.
 */
uint8_t vault_str_to_purpose_type( const char *str, uint32_t *purpose, uint32_t *coin_type );

/**
 * @brief Convenience function for app unit testing to set the vault according
 * to the derivation path string.
 *
 * No callback since it's always successful (Or will call TEST_FAIL).
 * Blocks until derivation is complete.
 *
 * To be ONLY used in a Unity Unit Test.
 *
 * On consumer builds, this function does nothing.
 *
 * @param[in] str Input path string like "44'/165'"
 * @param[in] bip32_key Input bip32 key like "ed25519 seed"
 */
void vault_set_unit_test( const char *str, const char *bip32_key );

#endif
