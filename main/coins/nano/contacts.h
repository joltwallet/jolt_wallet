/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#ifndef __JOLT_NANO_CONTACTS_H__
#define __JOLT_NANO_CONTACTS_H__

#include "nano_lib.h"

bool nano_set_contact_public(const uint256_t public_key, const int index);
bool nano_set_contact_name(const char *buf, const int index);
bool nano_get_contact_public(uint256_t public_key, const int index);
bool nano_get_contact_name(char *buf, size_t buf_len, const int index);
void nano_erase_contact(const int index);

#endif
