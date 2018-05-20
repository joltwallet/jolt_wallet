#ifndef __JOLT_NANO_CONFIRMATION_H__
#define __JOLT_NANO_CONFIRMATION_H__

#include "nano_lib.h"

bool nano_confirm_block(menu8g2_t *prev_menu, nl_block_t *head_block, nl_block_t *new_block);

#endif
