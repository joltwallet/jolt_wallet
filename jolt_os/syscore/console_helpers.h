#ifndef JOLT_SYSCORE_CONSOLE_HELPERS_H__
#define JOLT_SYSCORE_CONSOLE_HELPERS_H__

#include "stdint.h"
#include "stdbool.h"

bool console_check_range_argc(uint8_t argc, uint8_t min, uint8_t max);
bool console_check_equal_argc(uint8_t argc, uint8_t expected);

#endif
