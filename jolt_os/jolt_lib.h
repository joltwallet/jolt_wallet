/**
 * @file jolt_lib.h
 * @brief Primary JoltOS library for developing applications
 * @author Brian Pugh
 * @bugs
 *     * Currently versioning isn't used.
 */

#ifndef JOLT_LIB_H__
#define JOLT_LIB_H__

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
    #include "hal/display.h"
    #include "hal/storage/storage.h"
    #include "jolt_gui/jolt_gui.h"
    #include "jolt_helpers.h"
    #include "json_config.h"
    #include "linenoise/linenoise.h"
    #include "math.h"
    #include "mbedtls/bignum.h"
    #include "qrcode.h"
    #include "sodium.h"
    #include "sodium/crypto_verify_32.h"
    #include "sodium/private/curve25519_ref10.h"
    #include "syscore/cli.h"
    #include "syscore/cli_helpers.h"
    #include "syscore/cli_sub.h"
    #include "syscore/filesystem.h"
    #include "syscore/https.h"
    #include "vault.h"

#endif  // ESP_PLATFORM

/**
 * @brief Whether this was compiled for development or release
 */
enum {
    JOLT_VERSION_RELEASE = 0,
    JOLT_VERSION_DEV     = 1,
};
typedef uint8_t jolt_release_type_t;

enum {
    JOLT_HW_JOLT = 0,
};
typedef uint8_t jolt_hw_type_t;

typedef struct jolt_version_t {
    union {
        struct {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
            uint8_t padding; /**< explicit 0-padding */
        };
        uint32_t version;  // For easy comparison
    };
    jolt_release_type_t release;
} jolt_version_t;

extern const char *JOLT_OS_COMMIT;             /**< JoltOS commit */
extern const jolt_version_t JOLT_OS_VERSION;   /**< JoltOS version */
extern const jolt_version_t JOLT_JELF_VERSION; /**< Used to determine app compatibility */
extern const jolt_version_t JOLT_HW_VERSION;   /**< To check hardware compatability */

extern const jelfLoaderEnv_t jelf_loader_env;

#define cJSON_Get cJSON_GetObjectItemCaseSensitive
#define HARDEN    0x80000000

#endif
