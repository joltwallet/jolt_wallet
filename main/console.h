#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

void initialize_console();
void menu_console(menu8g2_t *prev);
bool console_check_argc(uint8_t argc, uint8_t limit);
volatile TaskHandle_t *start_console();

#endif
