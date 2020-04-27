#!/usr/bin/python3

'''
Compresses firmware for upload (reduces upload time by about 40%)
If no arguments are present, will default to the jolt_os.bin file in the build
directory.

The resulting compressed binary will have the same path as the input file, but
with a '.gz' extension.
'''

import argparse
import zlib
import os, sys
import logging

# Configure logging
logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)
log = logging.getLogger('elf2jelf')

def compress_data(data):
    w_bits = 12
    level = zlib.Z_BEST_COMPRESSION
    print("Compressing at level %d with window (dict) size %d" % (level, 2**w_bits))
    compressor = zlib.compressobj(level=level, method=zlib.DEFLATED,
            wbits=w_bits, memLevel=zlib.DEF_MEM_LEVEL, strategy=zlib.Z_DEFAULT_STRATEGY)
    compressed_data = compressor.compress(data)
    compressed_data += compressor.flush()
    compress_percentage = 100*(1-(len(compressed_data)/len(data)))
    print("Compressed data to %d bytes (%.2f%% smaller)" % \
            (len(compressed_data), compress_percentage) )
    return compressed_data

def parse_args():
    this_path = os.path.abspath(__file__)
    default_elf_fn = os.path.join(this_path, '..', 'build', 'jolt_os.bin')

    parser = argparse.ArgumentParser()
    parser.add_argument('--input', type=str, default=default_elf_fn,
            help='ESP32 binary file to compress.')
    parser.add_argument('--verbose', '-v', type=str, default='INFO',
            help='''
            Valid options:
            SILENT
            INFO
            DEBUG
            ''')
    args = parser.parse_args()
    dargs = vars(args)

    global log
    logging_level = args.verbose.upper()
    if logging_level == 'INFO':
        log.setLevel(logging.INFO)
    elif logging_level == 'DEBUG':
        log.setLevel(logging.DEBUG)
    else:
        raise("Invalid Logging Verbosity")

    return (args, dargs)

def main():
    args, dargs = parse_args()

    if not os.path.isfile(args.input):
        log.error("Can't find ELF file.")
        sys.exit(-1)

    with open(args.input, 'rb') as f:
        data = f.read()

    compressed_data = compress_data(data)

    elf_compressed_fn = args.input + '.gz'
    with open(elf_compressed_fn, 'wb') as f:
        f.write(compressed_data)

if __name__ == "__main__":
    main()
