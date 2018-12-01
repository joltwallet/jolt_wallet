#ifndef JELFLOADER_H__
#define JELFLOADER_H__

#include <stdio.h>
#include <stdint.h>

#if CONFIG_ELFLOADER_POSIX
    #define LOADER_FD_T FILE *
#else
    #define LOADER_FD_T void*
#endif

typedef struct {
    void **exported;            /*!< Pointer to exported symbols array */
    unsigned int exported_size; /*!< Elements on exported symbol array */
} jelfLoaderEnv_t;

typedef struct jelfLoaderContext_t jelfLoaderContext_t;

int jelfLoaderRun(jelfLoaderContext_t *ctx, int argc, char **argv);
int jelfLoaderRunAppMain(jelfLoaderContext_t *ctx);
int jelfLoaderRunConsole(jelfLoaderContext_t *ctx, int argc, char **argv);

jelfLoaderContext_t *jelfLoaderInit(LOADER_FD_T fd, const char *name, const jelfLoaderEnv_t *env);
jelfLoaderContext_t *jelfLoaderLoad(jelfLoaderContext_t *ctx);
jelfLoaderContext_t *jelfLoaderRelocate(jelfLoaderContext_t *ctx);

void jelfLoaderFree( jelfLoaderContext_t *ctx );

#if CONFIG_JELFLOADER_PROFILER_EN
/* Sets all profiler variables to 0 */
void jelfLoaderProfilerReset();

/* Prints the profiler results to uart console */
void jelfLoaderProfilerPrint();
#endif // CONFIG_ELFLOADER_PROFILER_EN

#endif
