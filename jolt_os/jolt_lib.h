#ifndef __JOLT_LIB_H__
#define __JOLT_LIB_H__

/* It's not excellent practice to have all these in a header file like this,
 * but it makes app development significantly easier */

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
#include "syscore/https.h"

#endif // ESP_PLATFORM

enum _jolt_release_type_t {
    JOLT_VERSION_RELEASE = 0,
    JOLT_VERSION_DEV = 1,
} release_type_t;
typedef uint8_t jolt_release_type_t;

typedef struct jolt_version_t {
    union {
        struct {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
            uint8_t padding;
        };
        uint32_t version; // For easy comparison
    };
    jolt_release_type_t release;
} jolt_version_t;

extern const jolt_version_t JOLT_VERSION;

extern const jelfLoaderEnv_t jelf_loader_env;

#endif
