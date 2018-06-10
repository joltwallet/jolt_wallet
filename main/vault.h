/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef __JOLT_VAULT_H__
#define __JOLT_VAULT_H__

#include "nvs.h"
#include "nano_lib.h"

/* Structure to store anything that if modified could perform something
 * malicious
 */
typedef struct vault_t{
    char mnemonic[MNEMONIC_BUF_LEN];
    uint512_t master_seed;
    bool valid;
} vault_t;

/* Add RPC command types here. If they are coin specific, it must be specified
 * between dummy commands "COIN_START" and "COIN_END". */
typedef enum vault_rpc_type_t {
    SYSCORE_START = 0,
    SYSCORE_WIFI_UPDATE,
    SYSCORE_END,

    NANO_START,
    NANO_BLOCK_SIGN, // Signs passed in block
    NANO_PUBLIC_KEY, // Derive public key at index
    NANO_CONTACT_UPDATE, // Update stored contact
    NANO_END

} vault_rpc_type_t;

typedef enum vault_rpc_response_t {
    RPC_UNLOCK = -1,
    RPC_SUCCESS = 0, // Anything below RPC_SUCCESS is considered a success
    RPC_FAILURE,
    RPC_CANCELLED,
    RPC_UNDEFINED,
    RPC_QUEUE_FULL
} vault_rpc_response_t;

typedef struct vault_rpc_t {
    enum vault_rpc_type_t type;
    uint64_t timestamp;
    QueueHandle_t response_queue;
    union{
        /* SYSCORE START */
        struct{
            char *mnemonic;
        } syscore_mnemonic_restore;
        struct{
            char *ssid;
            char *pass;
        } syscore_wifi_update;
        /* SYSCORE END */

        /* NANO START */
        struct{
            struct nl_block_t block;
            uint32_t index;
        } nano_public_key;
        struct{
            struct nl_block_t block;
            uint32_t index;
            struct nl_block_t frontier;
        } nano_block_sign;
        struct{
            char *name;
            uint8_t index;
            uint8_t *public; // pointer to public key
        } nano_contact_update;
        /* NANO END */
    };
} vault_rpc_t;

vault_rpc_response_t vault_rpc(vault_rpc_t *rpc);
nl_err_t vault_init();
void vault_task(void *vault_in);
nl_err_t init_nvm_namespace(nvs_handle *nvs_h, const char *namespace);

#endif
