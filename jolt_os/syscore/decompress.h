/**
 * @file decompress.h
 * @brief More convenient functions leveraging the minimal built-in miniz.
 *
 * Note: All compression/decompression in JoltOS operates on a 4KB dictionary size.
 * Typical ZLIB compression uses a 32KB dictionary size. Empirically, in typical
 * Jolt use-cases, this reduced dictionary size worsens compression by 1~2%. However,
 * with a reduced dictionary size, we only need 4KB of RAM instead of 32KB.
 *
 * Example on how to compress data in python with a 4KB dictionary:
 *
       import zlib
       def compress_data(data):
           w_bits = 12
           level = zlib.Z_BEST_COMPRESSION
           compressor = zlib.compressobj(level=level, method=zlib.DEFLATED,
                   wbits=w_bits, memLevel=zlib.DEF_MEM_LEVEL, strategy=zlib.Z_DEFAULT_STRATEGY)
           compressed_data = compressor.compress(data)
           compressed_data += compressor.flush()
           return compressed_data
 *
 * @author Brian Pugh
 */

#ifndef JOLT_SYSCORE_DECOMPRESS_H
#define JOLT_SYSCORE_DECOMPRESS_H

#include "jolttypes.h"

/**
 * @brief opague decompression object
 */
typedef struct decomp_t decomp_t;

/**
 * @brief Allocates and returns a decompressor object
 * @param[in] writer a writer function to handle the decompression stream.
 * @param[in] cookie user-defined metadata thats passed into the writer function
 * @return initialized decompression object
 */
decomp_t *decompress_obj_init( write_fun_t writer, void *cookie );

/**
 * @brief Decompresses data into an internal buffer, writing to file if necessary.
 * @param[in, out] d initialized decompression object
 * @param[in] data compressed chunk of data
 * @param[in] len length of the fed-in compressed chunk of data
 * @return amount of data decompressed in this chunk. Returns -1 on error.
 * */
int decompress_obj_chunk( decomp_t *d, uint8_t *data, size_t len );

/**
 * @brief Flushes remaining buffer to file, frees allocated space.
 * Does NOT close or do anything to clean up cookie/writer things.
 * param[in] d decompressor object to deallocate.
 */
void decompress_obj_del( decomp_t *d );

#endif
