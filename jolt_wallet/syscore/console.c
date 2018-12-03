/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "sodium.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "esp_console.h"
#include "driver/uart.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include "bipmnemonic.h"

#include "filesystem.h"
#include "launcher.h"

#include "jolt_globals.h"
#include "console.h"
#include "vault.h"
#include "jolt_helpers.h"
#include "hal/radio/wifi.h"
#include "hal/storage/storage.h"
#include "sodium.h"

#include "lvgl.h"
#include "jolt_gui/jolt_gui.h"

#include "../console.h"

static const char* TAG = "console_syscore";

static uint256_t app_key;

static lv_action_t set_app_key_back_cb(lv_obj_t *btn) {
    jolt_gui_scr_del();
    return LV_RES_INV;
}

static lv_action_t set_app_key_enter_cb(lv_obj_t *btn) {
    /* Todo: loading-like screen */
    storage_factory_reset(); // todo: uncomment when bug resolved
    if(!storage_set_blob(app_key, sizeof(app_key), "user", "app_key")){
        printf("Error setting app_key.\n");
    }
    printf("Successfully set App Key.\n");
    esp_restart();
    return LV_RES_OK;
}

static int cmd_app_key(int argc, char** argv){
    int return_code = 0;

    /* Input Validation */
    if( !console_check_range_argc(argc, 1, 2) ) {
        return_code = 1;
        goto exit;
    }

    /* Print App Key */
    if( 1 == argc ) {
        uint256_t approved_pub_key;
        hex256_t pub_key_hex;
        size_t required_size;
        if( !storage_get_blob(NULL, &required_size, "user", "app_key") ) {
            printf("Stored App Key not found\n");
            return_code = 4;
            goto exit;
        }
        if( sizeof(approved_pub_key) != required_size ||
                !storage_get_blob(approved_pub_key, &required_size,
                    "user", "app_key")) {
            printf("Stored App Key Blob doesn't have expected len.\n");
            return_code = 5;
            goto exit;
        }
        sodium_bin2hex(pub_key_hex, sizeof(pub_key_hex),
                approved_pub_key, sizeof(approved_pub_key));
        printf("App Key: %s\n", pub_key_hex);
        goto exit;
    }

    /* Set App Key */
    if( strlen(argv[1]) != 64 ) {
        printf("App Key must be 64 characters long in hexadecimal\n");
        return_code = 2;
        goto exit;
    }
    ESP_ERROR_CHECK(sodium_hex2bin(app_key, sizeof(app_key), argv[1], 64,
                NULL, NULL, NULL));

    /* Make sure we are not in an app */
    if( NULL != jolt_gui_store.app.ctx ) {
        printf("Cannot set app key while an app is running.\n");
        return_code = 3;
        goto exit;
    }

    /* Display Text; Pressing any button returns to previous screen */
    lv_obj_t *scr;
    char body[400];
    snprintf(body, sizeof(body), "WARNING: This will perform a factory reset.\nSet app public key to: \n%s ?", argv[1]);

    /* Prompt user */
    jolt_gui_sem_take();
    scr = jolt_gui_scr_text_create("Set App Key", body);
    jolt_gui_scr_set_back_action(scr, set_app_key_back_cb);
    jolt_gui_scr_set_enter_action(scr, set_app_key_enter_cb);
    jolt_gui_sem_give();
    // todo: delay until the user presses back instead of returning immediately
exit:
    return return_code;
}

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
        return return_code;
}

static int mnemonic_restore(int argc, char** argv) {
    const char title[] = "Restore";
    const char prompt[] = "Enter Mnemonic Word: ";
    int return_code = 0;
    char *line;
    CONFIDENTIAL char user_words[24][11];
    CONFIDENTIAL uint8_t index[24];

#if 0
    if( !menu_confirm_action(menu, "Begin mnemonic restore?") ) {
        return_code = -1;
        goto exit;
    }
#endif

    // Generate Random Order for user to input mnemonic
    for(uint8_t i=0; i< sizeof(index); i++){
        index[i] = i;
    }
    shuffle_arr(index, sizeof(index));

    //loading_enable();
    for(uint8_t i=0; i < sizeof(index); i++){
        uint8_t j = index[i];
        // Humans like 1-indexing
        char buf[10];
        snprintf(buf, sizeof(buf), "Word %d", j + 1);
        //loading_text_title(buf, title);

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
    //loading_disable();

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
#if 0
    if( !entry_verify_pin(&menu, pin_hash) ) {
        return_code = -2;
        goto exit;
    }
#endif

#if 0
    if( !menu_confirm_action(menu, "Save restored mnemonic and reboot? CAN NOT BE UNDONE.") ) {
        return_code = -1;
        goto exit;
    }
#endif

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
        return return_code;
}

static int jolt_cast(int argc, char** argv) {
    /* (url, path, port) */
    int return_code;
    char buf[100];

    // Check if number of inputs is correct
    if( !console_check_equal_argc(argc, 4) ) {
        return_code = 1;
        goto exit;
    }
    uint16_t port = atoi(argv[3]);

    // Confirm Inputs
    snprintf(buf, sizeof(buf), "Update jolt_cast server domain to:\n%s", argv[1]);
#if 0
    if( !menu_confirm_action(menu, buf) ) {
        return_code = -1;
        goto exit;
    }
    snprintf(buf, sizeof(buf), "Update jolt_cast server path to:\n%s", argv[2]);
    if( !menu_confirm_action(menu, buf) ) {
        return_code = -1;
        goto exit;
    }
    snprintf(buf, sizeof(buf), "Update jolt_cast server port to:\n%d", port);
    if( !menu_confirm_action(menu, buf) ) {
        return_code = -1;
        goto exit;
    }
#endif

    // Store User Values
    storage_set_str(argv[1], "user", "jc_domain");
    storage_set_str(argv[2], "user", "jc_path");
    storage_set_u16(port, "user", "jc_port");
    
    esp_restart();

    exit:
        return return_code;
}

static int cmd_reboot(int argc, char** argv) {
    esp_restart();
    return 0;
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

    cmd = (esp_console_cmd_t) {
        .command = "reboot",
        .help = "Reboot device.",
        .hint = NULL,
        .func = &cmd_reboot,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "app_key",
        .help = "Sets app public key. WILL ERASE ALL DATA.",
        .hint = NULL,
        .func = &cmd_app_key,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    console_syscore_fs_register();
    console_syscore_launcher_register();
}
