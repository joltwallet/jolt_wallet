# distutils: libraries = sodium c
# distutils: include_dirs = ../components/jolt-types/include ../jolt_os/jelf_loader/src ../jolt_os/jelf_loader/include ../jolt_os
# distutils: sources = ../jolt_os/jelf_loader/src/loader.c ../jolt_os/jelf_loader/src/unaligned.c

from libc.stdio cimport FILE, fopen
from libc.stdint cimport uint8_t, uint16_t, uint32_t

cdef extern from "jelfloader.h":
    struct jelfLoaderContext_t:
        pass
    struct jelfLoaderEnv_t:
        pass

    int jelfLoaderRun(jelfLoaderContext_t *ctx, int argc, char **argv)
    int jelfLoaderRunAppMain(jelfLoaderContext_t *ctx)
    int jelfLoaderRunConsole(jelfLoaderContext_t *ctx, int argc, char **argv)

    jelfLoaderContext_t *jelfLoaderInit(FILE *fd, const char *name, const jelfLoaderEnv_t *env)
    jelfLoaderContext_t *jelfLoaderLoad(jelfLoaderContext_t *ctx)
    jelfLoaderContext_t *jelfLoaderRelocate(jelfLoaderContext_t *ctx)

    void jelfLoaderFree( jelfLoaderContext_t *ctx )

    bint jelfLoaderSigCheck(jelfLoaderContext_t *ctx)
    uint8_t *jelfLoaderGetHash(jelfLoaderContext_t *ctx)

    void jelfLoaderProfilerReset()
    void jelfLoaderProfilerPrint()

cdef extern from "sodium.h":
    int sodium_init()

cdef extern from "jolt_lib.h":
    jelfLoaderEnv_t env

def jelf_loader_hash(fn: bytes, name_to_sign: bytes):
    if sodium_init() == -1:
        return;

    cdef jelfLoaderContext_t *ctx;

    cdef FILE *fd = fopen(fn, "rb");

    jelfLoaderProfilerReset();
    ctx = jelfLoaderInit(fd, name_to_sign, &env);
    jelfLoaderLoad(ctx);
    jelfLoaderRelocate(ctx);
    jelfLoaderProfilerPrint();
    return jelfLoaderGetHash(ctx)[:64];
