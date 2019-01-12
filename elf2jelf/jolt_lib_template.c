/* All of the functions available to an app.
 *
 * DO NOT CHANGE THIS FILE (jolt_lib.h) directly, either change:
 *     * elf2jelf/jolt_lib_template.h
 *     * elf2jelf/export_list.txt 
 *
 * jolt_lib.h will automatically be updated upon make
 * */

#include "jolt_lib.h"

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

const jelfLoaderEnv_t jelf_loader_env = {
    .exported = exports,
    .exported_size = sizeof(exports) / sizeof(*exports)
};
