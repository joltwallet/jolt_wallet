/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "esp_log.h"

#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "linenoise/linenoise.h"

#include "jolt_helpers.h"
#include "syscore/launcher.h"

#include "syscore/cli_sub.h"

static void jolt_cli_sub_cmd_help(jolt_cli_sub_t *subconsole);

static const char TAG[] = "syscore/cli_sub";

jolt_cli_sub_t *jolt_cli_sub_init() {
    jolt_cli_sub_t *subconsole;
    subconsole = calloc(1, sizeof(jolt_cli_sub_t));
    return subconsole;
}

int jolt_cli_sub_cmd_register(jolt_cli_sub_t *subconsole, esp_console_cmd_t *cmd) {
    jolt_cli_sub_t *new;
    jolt_cli_sub_t *current = subconsole;

    if( NULL==subconsole || NULL==cmd ) return -1;

    ESP_LOGD(TAG, "Registering cmd \"%s\"", cmd->command);

    if( NULL != current->cmd.func ) { // check if this cmd is populated
        new = jolt_cli_sub_init();
        if( NULL == new ) {
            return -1;
        }
        for(;;){
            if( NULL == current->next ){
                break;
            }
            current = current->next;
        }
        current->next = new;
    }
    else{
        new = current;
    }
    memcpy(&new->cmd, cmd, sizeof(esp_console_cmd_t));
    return 0;
}

static void jolt_cli_sub_cmd_help(jolt_cli_sub_t *subconsole) {
    jolt_cli_sub_t *current = subconsole;
    printf("Help:\n");
    while( current ) {
        printf("    %s - %s\n", current->cmd.command, current->cmd.help);
        current = current->next;
    }
}

int jolt_cli_sub_cmd_run(jolt_cli_sub_t *subconsole, uint8_t argc, char **argv) {
    jolt_cli_sub_t *current = subconsole;
    launch_inc_ref_ctr();
    if( argc > 0 ) {
        if( 0 == strcmp(argv[0], "help") ) {
            jolt_cli_sub_cmd_help(subconsole);
            return 0;
        }

        while( current ) {
            if( 0 == strcmp(argv[0], current->cmd.command) ) {
                // todo: shuttle this to as a bg job
                //jolt_bg_create( subconsole_execute, void *param, lv_obj_t *scr);
                return (current->cmd.func)(argc, argv);
            }
            current = current->next;
        }
    }
    printf("Command not found.\n");
    jolt_cli_sub_cmd_help(subconsole);
    return -100;
}

void jolt_cli_sub_cmd_free(jolt_cli_sub_t *subconsole) {
    jolt_cli_sub_t *current = subconsole;
    while( current ) {
        jolt_cli_sub_t *next = current->next;
        free(current);
        current = next;
    }
}

