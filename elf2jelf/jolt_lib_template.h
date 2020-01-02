/* All of the functions available to an app.
 *
 * DO NOT CHANGE THIS FILE (jolt_lib.h) directly, either change:
 *     * elf2jelf/jolt_lib_template.h
 *     * elf2jelf/export_list.txt 
 *
 * jolt_lib.c will automatically be updated upon make
 * */

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

#include "jelf_loader/include/jelfloader.h"

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
    #include "sdkconfig.h"
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
    #include "syscore/launcher.h"
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
extern const jolt_version_t JOLT_HW_VERSION;   /**< To check hardware compatability */

#define cJSON_Get cJSON_GetObjectItemCaseSensitive
#define HARDEN    0x80000000

#if JOLT_OS

/**< Used to determine app compatibility */
static const jolt_version_t JOLT_JELF_VERSION = {
    .major = %d,
    .minor = %d,
    .patch = %d,
    .release = %s,
};


extern int64_t __absvdi2(int64_t x);
extern int __absvsi2(int x);
extern double __adddf3(double x, double y);
extern float __addsf3(float x, float y);
extern int64_t __addvdi3(int64_t x, int64_t y);
extern int __addvsi3(int x, int y);
extern int64_t __ashldi3(int64_t x, int y);
extern int64_t __ashrdi3(int64_t x, int y);
extern int64_t __bswapdi2(int64_t x);
extern int32_t __bswapsi2(int32_t x);
extern int64_t __clrsbdi2(int64_t x);
extern int __clrsbsi2(int x);
extern int __clzdi2(int64_t x);
extern int __clzsi2(int x);
extern int __cmpdi2(int64_t x, int64_t y);
extern int __ctzdi2(uint64_t x);
extern int __ctzsi2(unsigned x);
extern complex double __divdc3(double a, double b, double c, double d);
extern double __divdf3(double x, double y);
extern int64_t __divdi3(int64_t x, int64_t y);
extern complex float __divsc3(float a, float b, float c, float d);
extern float __divsf3(float x, float y);
extern int __divsi3(int x, int y);
extern int __eqdf2(double x, double y);
extern int __eqsf2(float x, float y);
extern double __extendsfdf2(float x);
extern int __ffsdi2(uint64_t x);
extern int __ffssi2(unsigned x);
extern int64_t __fixdfdi(double x);
extern int __fixdfsi(double x);
extern int64_t __fixsfdi(float x);
extern int __fixsfsi(float x);
extern unsigned __fixunsdfsi(double x);
extern uint64_t __fixunssfdi(float x);
extern unsigned __fixunssfsi(float x);
extern double __floatdidf(int64_t);
extern float __floatdisf(int64_t);
extern double __floatsidf(int x);
extern float __floatsisf(int x);
extern double __floatundidf(uint64_t x);
extern float __floatundisf(uint64_t x);
extern double __floatunsidf(unsigned x);
extern float __floatunsisf(unsigned x);
extern int __gedf2(double x, double y);
extern int __gesf2(float x, float y);
extern int __gtdf2(double x, double y);
extern int __gtsf2(float x, float y);
extern int __ledf2(double x, double y);
extern int __lesf2(float x, float y);
extern int64_t __lshrdi3(int64_t x, int y);
extern int __ltdf2(double x, double y);
extern int __ltsf2(float x, float y);
extern int64_t __moddi3(int64_t x, int64_t y);
extern int __modsi3(int x, int y);
extern complex double __muldc3(double a, double b, double c, double d);
extern double __muldf3(double x, double y);
extern int64_t __muldi3(int64_t x, int64_t y);
extern complex float __mulsc3 (float a, float b, float c, float d);
extern float __mulsf3 (float a, float b);
extern int __mulsi3(int x, int y);
extern int __mulvdi3(int64_t x, int64_t y);
extern int __mulvsi3(int x, int y);
extern int __nedf2(double x, double y);
extern double __negdf2(double x);
extern int64_t __negdi2(int64_t x);
extern float __negsf2(float x);
extern int64_t __negvdi2(int64_t x);
extern int __negvsi2(int x);
extern int __nesf2(float x, float y);
extern int __paritysi2(unsigned x);
extern int __popcountdi2(uint64_t);
extern int __popcountsi2(unsigned x);
extern double __powidf2(double x, int y);
extern float __powisf2(float x, int y);
extern double __subdf3(double x, double y);
extern float __subsf3(float x, float y);
extern int64_t __subvdi3(int64_t x, int64_t y);
extern int __subvsi3(int x, int y);
extern float __truncdfsf2(double x);
extern int __ucmpdi2(uint64_t x, uint64_t y);
extern uint64_t __udivdi3(uint64_t x, uint64_t y);
extern uint64_t __udivmoddi4(uint64_t x, uint64_t y, uint64_t* z);
extern unsigned __udivsi3(unsigned x, unsigned y);
extern uint64_t __umoddi3(uint64_t x, uint64_t y);
extern unsigned __umodsi3(unsigned x, unsigned y);
extern uint64_t __umulsidi3(unsigned x, unsigned y);
extern int __unorddf2(double x, double y);
extern int __unordsf2(float x, float y);

#if CONFIG_COMPILER_STACK_CHECK
extern void *__stack_chk_fail;
extern void *__stack_chk_guard;
#else  // CONFIG_COMPILER_STACK_CHECK
static inline void __stack_chk_fail (void) { return; }
void *__stack_chk_guard = NULL;
#endif  // CONFIG_COMPILER_STACK_CHECK

#define EXPORT_SYMBOL(x) &x

/**
 * @brief Export functions to be used in applications.
 *
 * This order is very important; only *append* fuctions
 */
static const void *exports[] = {
%s
};

#else  // JOLT_OS

static const jolt_version_t JOLT_JELF_VERSION = { 0 };

/* Dummy place holder */
static const void *exports[%d] = { 0 };

#endif  // JOLT_OS 

static const jelfLoaderEnv_t jelf_loader_env = {
    .exported = exports,
    .exported_size = sizeof(exports) / sizeof(*exports)
};

#endif  // JOLT_LIB_H__
