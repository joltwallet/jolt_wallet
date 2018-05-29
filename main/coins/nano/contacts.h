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


#ifndef __JOLT_NANO_CONTACTS_H__
#define __JOLT_NANO_CONTACTS_H__

#include "nano_lib.h"

bool nano_set_contact_public(const uint256_t public_key, const int index);
bool nano_set_contact_name(const char *buf, const int index);
bool nano_get_contact_public(uint256_t public_key, const int index);
bool nano_get_contact_name(char *buf, size_t buf_len, const int index);
void nano_erase_contact(const int index);

#endif
