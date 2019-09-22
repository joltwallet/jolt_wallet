#include "syscore/encoding.h"

int jolt_encoding_rle_encode(uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len) {
#if 0
    size_t out_idx = 0, in_idx = 0;

    if(in_len == 0) return -1;

    uint8_t prev = in[0];
    uint8_t prev_counter = 0;
    for( size_t in_idx=1; in_idx < in_len; in_idx++) {
        if( in[in_idx] == prev ){
            prev_counter += 1;
            if (n== UINT8_MAX) {
                // flush
            }
        }
        else{
            prev_counter = 1;
            prev = in[in_idx];
        }
    }
#else
    return -1;
#endif
}

int jolt_encoding_rle_decode(uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len) {
    //TODO
    return -1;
}

