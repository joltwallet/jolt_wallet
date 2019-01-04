#!/usr/bin/python3

import zlib
import os

elf_fn = os.path.join('build', 'jolt_os.elf')
elf_compressed_fn = elf_fn + '.gz'

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

def main():
    if not os.path.isfile(elf_fn):
        print("Can't find ELF file.")
        return

    with open(elf_fn, 'rb') as f:
        data = f.read()
    compressed_data = compress_data(data)
    with open(elf_compressed_fn, 'wb') as f:
        f.write(compressed_data)

if __name__ == "__main__":
    main()
