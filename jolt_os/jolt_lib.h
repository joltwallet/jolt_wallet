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

    #include "app_cmd/contacts.h"
    #include "assert.h"
    #include "bipmnemonic.h"
    #include "cJSON.h"
    #include "complex.h"
    #include "ctype.h"
    #include "driver/uart.h"
    #include "errno.h"
    #include "esp_console.h"
    #include "esp_log.h"
    #include "esp_vfs_dev.h"
    #include "fenv.h"
    #include "float.h"
    #include "freertos/FreeRTOS.h"
    #include "freertos/queue.h"
    #include "freertos/task.h"
    #include "hal/display.h"
    #include "hal/storage/storage.h"
    #include "inttypes.h"
    #include "iso646.h"
    #include "jolt_crypto.h"
    #include "jolt_gui/jolt_gui.h"
    #include "jolt_hash.h"
    #include "jolt_helpers.h"
    #include "json_config.h"
    #include "limits.h"
    #include "locale.h"
    #include "math.h"
    #include "mbedtls/bignum.h"
    #include "newlib.h"
    #include "qrcode.h"
    #include "setjmp.h"
    #include "signal.h"
    #include "sodium.h"
    #include "sodium/private/curve25519_ref10.h"
    #include "stdalign.h"
    #include "stdarg.h"
    #include "stdatomic.h"
    #include "stdbool.h"
    #include "stddef.h"
    #include "stdint.h"
    #include "stdio.h"
    #include "stdlib.h"
    #include "stdnoreturn.h"
    #include "string.h"
    #include "syscore/cli.h"
    #include "syscore/cli_helpers.h"
    #include "syscore/cli_sub.h"
    #include "syscore/filesystem.h"
    #include "syscore/https.h"
    #include "tgmath.h"
    #include "time.h"
    #include "vault.h"
    #include "wchar.h"
    #include "wctype.h"

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
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    jolt_release_type_t release;
} jolt_version_t;

extern const jolt_version_t JOLT_OS_VERSION;   /**< JoltOS version */
extern const jolt_version_t JOLT_JELF_VERSION; /**< Used to determine app compatibility */
extern const jolt_version_t JOLT_HW_VERSION;   /**< To check hardware compatability */

extern const jelfLoaderEnv_t jelf_loader_env;

#define cJSON_Get cJSON_GetObjectItemCaseSensitive
#define HARDEN    0x80000000

#endif
