#ifndef __JOLT_NANO_CONFIRMATION_H__
#define __JOLT_NANO_CONFIRMATION_H__

#include "nano_lib.h"
#include "menu8g2.h"

bool nano_confirm_block(menu8g2_t *prev_menu, nl_block_t *head_block, nl_block_t *new_block);
bool nano_confirm_contact_update(const menu8g2_t *prev_menu, const char *name, const uint256_t public, const uint8_t index);

#endif
