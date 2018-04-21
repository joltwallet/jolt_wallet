#ifndef __NANORAY_GLOBALS_H__
#define __NANORAY_GLOBALS_H__

#include "u8g2.h"

#define GLOBAL

extern u8g2_t u8g2;
extern QueueHandle_t vault_queue;
extern QueueHandle_t input_queue;

#endif
