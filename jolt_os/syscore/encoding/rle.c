#include "syscore/encoding.h"

int jolt_encoding_rle_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
    size_t out_idx = 0;

    if( 0 == in_len || NULL == in ) return -1;

    uint8_t prev = in[0];
    uint8_t n    = 0;
    for( size_t in_idx = 0; in_idx < in_len; in_idx++ ) {
        if( in[in_idx] == prev ) {
            n++;
            if( UINT8_MAX == n) {
                /* Write Output */
                if( out && out_idx < out_len ) out[out_idx] = n;
                out_idx++;
                if( out && out_idx < out_len ) out[out_idx] = prev;
                out_idx++;
                n = 0;
                continue;
            }
            if( in_idx != in_len - 1 ) continue;
        }

        /* write sequence to output */
        if( n > 0 ) {
            if( out && out_idx < out_len ) out[out_idx] = n;
            out_idx++;
            if( out && out_idx < out_len ) out[out_idx] = prev;
            out_idx++;

            if(in[in_idx] != prev && in_idx == in_len - 1) {
                /* write final differing character to output */
                if( out && out_idx < out_len ) out[out_idx] = 1;
                out_idx++;
                if( out && out_idx < out_len ) out[out_idx] = in[in_idx];
                out_idx++;
                break;
            }
        }

        prev = in[in_idx];
        n    = 1;
    }
    return out_idx;
}

int jolt_encoding_rle_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
    size_t out_idx = 0;

    if( in_len % 2 != 0 || 0 == in_len || NULL == in ) return -1;

    for( size_t in_idx = 0; in_idx < in_len; in_idx += 2 ) {
        uint8_t n = in[in_idx];
        uint8_t c = in[in_idx + 1];
        for( uint8_t i = 0; i < n; i++ ) {
            if( out && out_idx < out_len ) out[out_idx] = c;
            out_idx++;
        }
    }
    return out_idx;
}
