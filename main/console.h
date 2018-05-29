#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

void initialize_console();
void menu_console(menu8g2_t *prev);
volatile TaskHandle_t *start_console();

#endif
