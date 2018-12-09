/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

#include "esp_console.h"
#include "lvgl/lvgl.h"

void console_init();
bool console_check_range_argc(uint8_t argc, uint8_t min, uint8_t max);
bool console_check_equal_argc(uint8_t argc, uint8_t expected);
volatile TaskHandle_t *console_start();

int jolt_cmd_process(char *line, FILE *in, FILE *out, FILE *err);

typedef struct subconsole_t {
    esp_console_cmd_t cmd;
    struct subconsole_t *next;
} subconsole_t;

extern QueueHandle_t jolt_cmd_queue;

subconsole_t *subconsole_cmd_init();
int subconsole_cmd_register(subconsole_t *subconsole, esp_console_cmd_t *cmd);
int subconsole_cmd_run(subconsole_t *subconsole, uint8_t argc, char **argv);
void subconsole_cmd_free(subconsole_t *subconsole);

#endif
