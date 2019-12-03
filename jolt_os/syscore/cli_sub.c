/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include "syscore/cli_sub.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "jolt_helpers.h"
#include "sodium.h"
#include "syscore/launcher.h"

static void jolt_cli_sub_cmd_help( jolt_cli_sub_t *subconsole );

static const char TAG[] = "syscore/cli_sub";

jolt_cli_sub_t *jolt_cli_sub_init()
{
    jolt_cli_sub_t *subconsole;
    subconsole = calloc( 1, sizeof( jolt_cli_sub_t ) );
    return subconsole;
}

int jolt_cli_sub_cmd_register( jolt_cli_sub_t *subconsole, esp_console_cmd_t *cmd )
{
    jolt_cli_sub_t *new;
    jolt_cli_sub_t *current = subconsole;

    if( NULL == subconsole || NULL == cmd ) return -1;

    ESP_LOGD( TAG, "Registering cmd \"%s\"", cmd->command );

    if( NULL != current->cmd.func ) {  // check if this cmd is populated
        new = jolt_cli_sub_init();
        if( NULL == new ) { return -1; }
        for( ;; ) {
            if( NULL == current->next ) { break; }
            current = current->next;
        }
        current->next = new;
    }
    else {
        new = current;
    }
    memcpy( &new->cmd, cmd, sizeof( esp_console_cmd_t ) );
    return 0;
}

static void jolt_cli_sub_cmd_help( jolt_cli_sub_t *subconsole )
{
    jolt_cli_sub_t *current = subconsole;
    printf( "Help:\n" );
    while( current ) {
        printf( "    %s - %s\n", current->cmd.command, current->cmd.help );
        current = current->next;
    }
}

int jolt_cli_sub_cmd_run( jolt_cli_sub_t *subconsole, uint8_t argc, const char **argv )
{
    jolt_cli_sub_t *current = subconsole;
    if( argc > 0 ) {
        if( 0 == strcmp( argv[0], "help" ) ) {
            jolt_cli_sub_cmd_help( subconsole );
            return 0;
        }

        while( current ) {
            if( 0 == strcmp( argv[0], current->cmd.command ) ) {
                int rc;
                rc = ( current->cmd.func )( argc, (char **)argv );
                return rc;
            }
            current = current->next;
        }
    }
    else {
        printf( "No arguments provided.\n" );
    }
    printf( "Command not found.\n" );
    return -100;
}

void jolt_cli_sub_cmd_free( jolt_cli_sub_t *subconsole )
{
    jolt_cli_sub_t *current = subconsole;
    while( current ) {
        jolt_cli_sub_t *next = current->next;
        free( current );
        current = next;
    }
}
