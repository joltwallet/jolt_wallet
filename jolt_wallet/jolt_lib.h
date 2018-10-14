#ifndef __JOLT_LIB_H__
#define __JOLT_LIB_H__

#include "elfloader.h"
#define EXPORT_SYMBOL(NAME) {#NAME, (void*) NAME}
#define EXPORT_SYMBOL_PTR(NAME) {#NAME, (void*) &NAME}
#include "elf_exports.h"
const ELFLoaderEnv_t env = { exports, sizeof(exports) / sizeof(*exports) };

#endif
