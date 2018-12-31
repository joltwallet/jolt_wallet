/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "sodium.h"

#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "esp_log.h"
#include "linenoise/linenoise.h"

#include "filesystem.h"
#include "launcher.h"

#include "jolt_globals.h"
#include "syscore/console.h"
#include "syscore/console_helpers.h"

#include "ota.h"

#include "../console.h"
#include "syscore/cmd/jolt_cmds.h"

static const char* TAG = "console_syscore";

void console_syscore_register() {
    esp_console_cmd_t cmd;

    cmd = (esp_console_cmd_t) {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &jolt_cmd_free,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "task_status",
        .help = "Memory usage of all running tasks.",
        .hint = NULL,
        .func = &jolt_cmd_task_status,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "top",
        .help = "CPU-Usage of Tasks.",
        .hint = NULL,
        .func = &jolt_cmd_top,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "wifi_update",
        .help = "Update WiFi SSID and Pass.",
        .hint = NULL,
        .func = &jolt_cmd_wifi_update,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "mnemonic_restore",
        .help = "Restore mnemonic seed.",
        .hint = NULL,
        .func = &jolt_cmd_mnemonic_restore,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "jolt_cast_update",
        .help = "Update jolt_cast (domain, path, port).",
        .hint = NULL,
        .func = &jolt_cmd_jolt_cast_update,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "reboot",
        .help = "Reboot device.",
        .hint = NULL,
        .func = &jolt_cmd_reboot,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "app_key",
        .help = "Sets app public key. WILL ERASE ALL DATA.",
        .hint = NULL,
        .func = &jolt_cmd_app_key,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "upload_firmware",
        .help = "Update JoltOS",
        .hint = NULL,
        .func = &jolt_cmd_upload_firmware,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    console_syscore_fs_register();
    console_syscore_launcher_register();
}
