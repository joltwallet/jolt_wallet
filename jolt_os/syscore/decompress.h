#ifndef JOLT_SYSCORE_DECOMPRESS_H
#define JOLT_SYSCORE_DECOMPRESS_H

#include "jolttypes.h"

typedef struct decomp_t decomp_t;

/* Allocates and returns a decompressor object */
decomp_t *decompress_obj_init( write_fun_t writer, void *cookie );

/* Decompresses data into internal buffer, writing to file if necessary.
 * Returns amount of data decompressed in this chunk.
 * Returns -1 on error.
 * */
int decompress_obj_chunk( decomp_t *d, uint8_t *data, size_t len );

/* Flushes remaining buffer to file, frees allocated space. Does NOT close or
 * do anything to clean up cookie/writer things.*/
void decompress_obj_del ( decomp_t *d);

#endif
