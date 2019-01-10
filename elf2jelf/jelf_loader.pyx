# distutils: libraries = sodium c
# distutils: include_dirs = ../components/jolt-types/include ../jolt_os/jelf_loader/src ../jolt_os/jelf_loader/include
# distutils: sources = ../jolt_os/jelf_loader/src/loader.c ../jolt_os/jelf_loader/src/unaligned.c

cdef extern from "jelfloader.h":
    void jelfLoaderHash(char *fn, char *fn_basename, int n_exports);

def jelfLoader(fn: bytes, fn_basename: bytes, n_exports: int):
    jelfLoaderHash(fn, fn_basename, n_exports)
