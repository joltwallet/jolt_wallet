/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "menu8g2.h"
#include <libwebsockets.h>
#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include "nano_lws.h"
#include "nano_parse.h"
#include "bipmnemonic.h"

#include "filesystem.h"
#include "launcher.h"

#include "../globals.h"
#include "../console.h"
#include "../vault.h"
#include "../helpers.h"
#include "../gui/gui.h"
#include "../gui/loading.h"
#include "../gui/statusbar.h"
#include "../gui/confirmation.h"
#include "../gui/entry.h"
#include "../radio/wifi.h"
#include "../hal/storage.h"

#include "../console.h"

static const char* TAG = "console_syscore";


static int free_mem(int argc, char** argv) {
    printf("Free: %d bytes\n", esp_get_free_heap_size());
    return 0;
}

static int task_status(int argc, char** argv) {
    /* Get Task Memory Usage */
    char pcWriteBuffer[1024];
    vTaskList( pcWriteBuffer );
    printf("B - Blocked | R - Ready | D - Deleted | S - Suspended\n"
           "Task            Status Priority High    Task #\n"
           "**********************************************\n");
    printf( pcWriteBuffer );
    return 0;
}

static int cpu_status(int argc, char** argv) {
    /* Gets Task CPU usage statistics */
    char pcWriteBuffer[1024];
    printf("Task            Abs Time (uS)           %%Time\n"
           "*********************************************\n");
    vTaskGetRunTimeStats( pcWriteBuffer );
    printf( pcWriteBuffer );
    return 0;
}

static int wifi_update(int argc, char** argv) {
    int return_code;
    SCREEN_SAVE;

    if( !console_check_range_argc(argc, 2, 3) ) {
        return_code = 1;
        goto exit;
    }

    bool update_success;
    if( 3 == argc ) {
        update_success = set_wifi_credentials(argv[1], argv[2]);
    }
    else {
        update_success = set_wifi_credentials(argv[1], "");
    }

    if( update_success ) {
        printf("Wifi Settings Updated. Restarting");
        return_code = 0;
        esp_restart();
    }
    else {
        printf("Error Updating WiFi Settings\n");
        return_code = 1;
        goto exit;
    }

    exit:
        SCREEN_RESTORE;
        return return_code;
}

static int mnemonic_restore(int argc, char** argv) {
    const char title[] = "Restore";
    const char prompt[] = "Enter Mnemonic Word: ";
    int return_code = 0;
    char *line;
    CONFIDENTIAL char user_words[24][11];
    CONFIDENTIAL uint8_t index[24];
    menu8g2_t menu;
    menu8g2_init(&menu, (u8g2_t *) &u8g2, input_queue, disp_mutex, NULL, statusbar_update);
    SCREEN_SAVE;

    if( !menu_confirm_action(&menu, "Begin mnemonic restore?") ) {
        return_code = -1;
        goto exit;
    }

    // Generate Random Order for user to input mnemonic
    for(uint8_t i=0; i< sizeof(index); i++){
        index[i] = i;
    }
    shuffle_arr(index, sizeof(index));

    loading_enable();
    for(uint8_t i=0; i < sizeof(index); i++){
        uint8_t j = index[i];
        // Humans like 1-indexing
        char buf[10];
        snprintf(buf, sizeof(buf), "Word %d", j + 1);
        loading_text_title(buf, title);

        line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        if (strcmp(line, "exit_restore") == 0){
            printf("Aborting mnemonic restore\n");
            linenoiseFree(line);
            return_code = 1;
            goto exit;
        }

        strlcpy(user_words[j], line, sizeof(user_words[j]));
        linenoiseFree(line);

        // verify its a word in the word list
        while(-1 == bm_search_wordlist(user_words[j], strlen(user_words[j]))) {
            printf("Invalid word\n");
            line = linenoise(prompt);
            if (line == NULL) { /* Ignore empty lines */
                continue;
            }
            if (strcmp(line, "exit_restore") == 0){
                printf("Aborting mnemonic restore\n");
                linenoiseFree(line);
                return_code = 1;
                goto exit;
            }

            strlcpy(user_words[j], line, sizeof(user_words[j]));
            linenoiseFree(line);
        }
    }
    sodium_memzero(index, sizeof(index));
    loading_disable();

    // Join Mnemonic into single buffer
    CONFIDENTIAL char mnemonic[BM_MNEMONIC_BUF_LEN];
    size_t offset=0;
    for(uint8_t i=0; i < sizeof(index); i++){
        strlcpy(mnemonic + offset, user_words[i], sizeof(mnemonic) - offset);
        offset += strlen(user_words[i]);
        mnemonic[offset++] = ' ';
    }
    mnemonic[offset - 1] = '\0'; //null-terminate, remove last space

    // prompt and verify new pin; result: pin_hash
    CONFIDENTIAL uint256_t pin_hash;
    if( !entry_verify_pin(&menu, pin_hash) ) {
        return_code = -2;
        goto exit;
    }

    if( !menu_confirm_action(&menu, "Save restored mnemonic and reboot? CAN NOT BE UNDONE.") ) {
        return_code = -1;
        goto exit;
    }

    CONFIDENTIAL uint256_t bin;
    jolt_err_t err = bm_mnemonic_to_bin(bin, sizeof(bin), mnemonic);

    storage_set_mnemonic(bin, pin_hash);
    sodium_memzero(bin, sizeof(bin));
    sodium_memzero(index, sizeof(index));
    sodium_memzero(mnemonic, sizeof(mnemonic));
    esp_restart();

    exit:
        sodium_memzero(index, sizeof(index));
        sodium_memzero(mnemonic, sizeof(mnemonic));
        SCREEN_RESTORE;
        return return_code;
}

static int jolt_cast(int argc, char** argv) {
    /* (url, path, port) */
    int return_code;
    char buf[100];
    menu8g2_t menu;
    menu8g2_init(&menu, (u8g2_t *) &u8g2, input_queue, disp_mutex, NULL, statusbar_update);
    SCREEN_SAVE;

    // Check if number of inputs is correct
    if( !console_check_equal_argc(argc, 4) ) {
        return_code = 1;
        goto exit;
    }
    uint16_t port = atoi(argv[3]);

    // Confirm Inputs
    snprintf(buf, sizeof(buf), "Update jolt_cast server domain to:\n%s", argv[1]);
    if( !menu_confirm_action(&menu, buf) ) {
        return_code = -1;
        goto exit;
    }
    snprintf(buf, sizeof(buf), "Update jolt_cast server path to:\n%s", argv[2]);
    if( !menu_confirm_action(&menu, buf) ) {
        return_code = -1;
        goto exit;
    }
    snprintf(buf, sizeof(buf), "Update jolt_cast server port to:\n%d", port);
    if( !menu_confirm_action(&menu, buf) ) {
        return_code = -1;
        goto exit;
    }

    // Store User Values
    storage_set_str(argv[1], "user", "jc_domain");
    storage_set_str(argv[2], "user", "jc_path");
    storage_set_u16(port, "user", "jc_port");
    
    esp_restart();

    exit:
        SCREEN_RESTORE;
        return return_code;
}

void console_syscore_register() {
    esp_console_cmd_t cmd;

    cmd = (esp_console_cmd_t) {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &free_mem,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "task_status",
        .help = "Memory usage of all running tasks.",
        .hint = NULL,
        .func = &task_status,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "top",
        .help = "CPU-Usage of Tasks.",
        .hint = NULL,
        .func = &cpu_status,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "wifi_update",
        .help = "Update WiFi SSID and Pass.",
        .hint = NULL,
        .func = &wifi_update,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "mnemonic_restore",
        .help = "Restore mnemonic seed.",
        .hint = NULL,
        .func = &mnemonic_restore,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "jolt_cast",
        .help = "Update jolt_cast (domain, path, port).",
        .hint = NULL,
        .func = &jolt_cast,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    console_syscore_fs_register();
    console_syscore_launcher_register();
}
