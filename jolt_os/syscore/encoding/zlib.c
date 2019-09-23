/*
 *@bugs
 *    * Haven't been able to test due to OOM errors while initializing mz_deflateInit2.
 */

#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "miniz.h"
#include "syscore/encoding.h"

#define ENABLE_ZLIB 0
#define WINDOW_BITS MZ_DEFAULT_WINDOW_BITS

#if ENABLE_ZLIB
static const char TAG[] = "syscore/encoding/zlib";
#endif

int jolt_encoding_zlib_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
#if ENABLE_ZLIB
    int res;
    size_t out_idx = 0;

    if( 0 == in_len || NULL == in ) return -1;

    mz_stream stream = {0};
    res = mz_deflateInit2( &stream, MZ_BEST_COMPRESSION, MZ_DEFLATED, WINDOW_BITS, 1, MZ_DEFAULT_STRATEGY );
    if( MZ_OK != res ) {
        ESP_LOGE( TAG, "Failed to initialize deflate (%s).", mz_error( res ) );
        return -1;
    }
    stream.next_in   = in;
    stream.avail_in  = in_len;
    stream.next_out  = out;
    stream.avail_out = out_len;

    if( NULL == out || 0 == out_len ) {
        /* Conservative estimate of output size */
        out_idx = mz_deflateBound( &stream, in_len );
    }
    else {
        /* Perform compression */
        do {
            res = mz_deflate( &stream, Z_NO_FLUSH );
        } while( res == MZ_OK );

        if( MZ_STREAM_END != res ) {
            ESP_LOGE( TAG, "%s", mz_error( res ) );
            out_idx = -1;
        }
        else {
            out_idx = stream.next_out - out;
        }

        out_idx = stream.next_out - out;
    }

    res = mz_deflateEnd( &stream );
    if( MZ_OK != res ) { ESP_LOGE( TAG, "Failed to deinit deflate (%s).", mz_error( res ) ); }

    return out_idx;
#else
    return -1;
#endif
}

int jolt_encoding_zlib_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
#if ENABLE_ZLIB
    int res;
    size_t out_idx = 0;

    if( 0 == in_len || NULL == in ) return -1;

    mz_stream stream = {0};
    res              = mz_inflateInit2( &stream, WINDOW_BITS );
    if( MZ_OK != res ) {
        ESP_LOGE( TAG, "Failed to initialize inflate (%s).", mz_error( res ) );
        return -1;
    }
    stream.next_in   = in;
    stream.avail_in  = in_len;
    stream.next_out  = out;
    stream.avail_out = out_len;

    if( NULL == out || 0 == out_len ) {
        /* Conservative estimate of output size */
        out_idx = -1;  // Cannot estimate output size
    }
    else {
        int res;

        /* Perform compression */
        do {
            res = mz_inflate( &stream, Z_NO_FLUSH );
        } while( res == MZ_OK );

        if( MZ_STREAM_END != res ) {
            ESP_LOGE( TAG, "%s", mz_error( res ) );
            out_idx = -1;
        }
        else {
            out_idx = stream.next_out - out;
        }
    }

    res = mz_deflateEnd( &stream );
    if( MZ_OK != res ) { ESP_LOGE( TAG, "Failed to deinit deflate (%s).", mz_error( res ) ); }

    return out_idx;
#else
    return -1;
#endif
}
