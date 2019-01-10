from setuptools import setup, find_packages
from setuptools.extension import Extension
from Cython.Build import cythonize
import os

import ipdb as pdb
# Unmangle env variables if part of ESP-IDF build
if 'CFLAGS' in os.environ and ('-DJOLT_OS' in os.environ['CFLAGS'] or '-DJOLT_APP' in os.environ['CFLAGS']:
    if 'HOSTAR' in os.environ:
        os.environ['AR'] = os.environ['HOSTAR']
    if 'HOSTCC' in os.environ:
        os.environ['CC'] = os.environ['HOSTCC']
    if 'HOSTLD' in os.environ:
        os.environ['LD'] = os.environ['HOSTLD']
    if 'HOSTOBJCOPY' in os.environ:
        os.environ['OBJCOPY'] = os.environ['HOSTOBJCOPY']
    del os.environ['CFLAGS']
    del os.environ['CPPFLAGS']
    del os.environ['CXX']
    del os.environ['CXXFLAGS']
    del os.environ['MAKEFLAGS']
    del os.environ['MAKEOVERRIDES']
    del os.environ['MFLAGS']

extensions = [
    Extension(
        "jelf_loader",
        ["jelf_loader.pyx"],
        language="c",
        define_macros = [
                ('ESP_PLATFORM', "0"),
                ('CONFIG_JELFLOADER_CACHE_LOCALITY', None),
                ('CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_N', "6"),
                ('CONFIG_JELFLOADER_CACHE_LOCALITY_CHUNK_SIZE', "2048"),
                ],
        extra_compile_args=[],
        extra_linker_args=[],
    ),
]

setup(
    name = "jelf_loader",
    ext_modules = cythonize(extensions, force=True)
)
