/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_CONSOLE_H__
#define __JOLT_CONSOLE_H__

#include "esp_console.h"
#include "lvgl/lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


typedef struct jolt_cmd_t {
    SemaphoreHandle_t complete;
    int return_value;
    char *data;
    FILE *fd_in;
    FILE *fd_out;
    FILE *fd_err;
} jolt_cmd_t;

void console_init();
volatile TaskHandle_t *console_start();

int jolt_cmd_process(char *line, FILE *in, FILE *out, FILE *err, bool block);
void jolt_cmd_del(jolt_cmd_t *cmd);

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
