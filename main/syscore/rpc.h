/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_SYSCORE_RPC_H__
#define __JOLT_SYSCORE_RPC_H__

#include "menu8g2.h"
#include "../vault.h"

vault_rpc_response_t rpc_syscore(vault_t *vault, vault_rpc_t *cmd, menu8g2_t *menu);

#endif
