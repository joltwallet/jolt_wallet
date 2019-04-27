/* All of the functions available to an app.
 *
 * DO NOT CHANGE THIS FILE (jolt_lib.h) directly, either change:
 *     * elf2jelf/jolt_lib_template.h
 *     * elf2jelf/export_list.txt 
 *
 * jolt_lib.h will automatically be updated upon make
 * */

#include "jolt_lib.h"
#include "sdkconfig.h"

#if JOLT_OS

const jolt_version_t JOLT_JELF_VERSION = {
    .major = %d,
    .minor = %d,
    .patch = %d,
    .release = %s,
};

extern void *__fixunsdfsi;
extern void *__floatunsidf;
extern void *__floatsidf;
extern void *__gtdf2;
extern void *__ltdf2;
extern void *__muldf3;

#if CONFIG_STACK_CHECK
extern void *__stack_chk_fail;
extern void *__stack_chk_guard;
#else
static inline void __stack_chk_fail (void) { return; }
void *__stack_chk_guard = NULL;
#endif

#define EXPORT_SYMBOL(x) &x

/**
 * @brief Export functions to be used in applications.
 *
 * This order is very important; only *append* fuctions
 */
static const void *exports[] = {
%s
};

#else

const jolt_version_t JOLT_JELF_VERSION = { 0 };

const char *JOLT_OS_COMMIT = NULL;

/* Dummy place holder */
static const void *exports[%d] = { 0 };

#endif

const jelfLoaderEnv_t jelf_loader_env = {
    .exported = exports,
    .exported_size = sizeof(exports) / sizeof(*exports)
};
