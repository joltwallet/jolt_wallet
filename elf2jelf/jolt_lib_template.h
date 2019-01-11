#ifndef __JOLT_LIB_H__
#define __JOLT_LIB_H__

/* All of the functions available to an app.
 *
 * DO NOT CHANGE THIS FILE (jolt_lib.h) directly, either change:
 *     * elf2jelf/jolt_lib_template.h
 *     * elf2jelf/export_list.txt 
 *
 * jolt_lib.h will automatically be updated upon make
 * */

#include "jelfloader.h"

#if ESP_PLATFORM

#include "bipmnemonic.h"
#include "cJSON.h"
#include "driver/uart.h"
#include "esp_console.h"
#include "esp_log.h"
#include "esp_vfs_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "linenoise/linenoise.h"
#include "math.h"
#include "mbedtls/bignum.h"
#include "nano_rest.h"
#include "qrcode.h"
#include "sodium.h"
#include "sodium/crypto_verify_32.h"
#include "sodium/private/curve25519_ref10.h"

#include "console.h"
#include "hal/storage/storage.h"
#include "hal/display.h"
#include "jolt_helpers.h"
#include "jolt_gui/jolt_gui.h"
#include "syscore/filesystem.h"
#include "vault.h"
#include "syscore/console_helpers.h"


#include "esp_http_client.h" // todo: replace with less open functions

#endif // ESP_PLATFORM

typedef enum release_type_t {
    JOLT_VERSION_RELEASE = 0,
    JOLT_VERSION_DEV = 1,
} release_type_t;

typedef struct jolt_version_t {
    union {
        struct {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
        };
        uint32_t version; // For easy comparison
    };
    release_type_t release;
} jolt_version_t;

extern const jolt_version_t JOLT_VERSION;

#if JOLT_OS

extern void *__floatsidf;
extern void *__gtdf2;
extern void *__ltdf2;
extern void *__muldf3;
extern void *__stack_chk_fail;
extern void *__stack_chk_guard;

#define EXPORT_SYMBOL(x) &x

/* This order is very important; only *append* fuctions */
static const void *exports[] = {
%s
};

#else

/* Dummy place holder */
static const void *exports[%d] = { 0 };

#endif

static const jelfLoaderEnv_t env = {
    .exported = exports,
    .exported_size = sizeof(exports) / sizeof(*exports)
};


#endif
