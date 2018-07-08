#ifndef __JOLT_LIB_H__
#define __JOLT_LIB_H__


#ifdef JOLTOS 

#include "loader.h"
#define EXPORT_SYMBOL(NAME) {#NAME, (void*) NAME}

const ELFLoaderSymbol_t exports[] = {
    EXPORT_SYMBOL( puts ),
    EXPORT_SYMBOL( crypto_generichash ),
    EXPORT_SYMBOL( sodium_bin2hex ),
    EXPORT_SYMBOL( memset )
};
const ELFLoaderEnv_t env = { exports, sizeof(exports) / sizeof(*exports) };

#else

extern void printf(char *, ...);
extern int crypto_generichash(unsigned char *, size_t, const unsigned char *, unsigned long long, const unsigned char *, size_t);
extern char *sodium_bin2hex(char * const, const size_t,

#endif



#endif
