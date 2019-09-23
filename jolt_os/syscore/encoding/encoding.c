#include "syscore/encoding.h"

int jolt_encoding_none_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
    if( NULL == in ) return -1;
    if( NULL == out || out_len < in_len ) goto exit;
    memcpy( out, in, in_len );

exit:
    return in_len;
}

int jolt_encoding_none_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
    /* It's an identity mapping, so encoder works as decoder */
    return jolt_encoding_none_encode( out, out_len, in, in_len );
}

jolt_encoding_t jolt_encoding_get_encoder( jolt_encoding_type_t encoding )
{
    switch( encoding ) {
        case JOLT_ENCODING_NONE: return jolt_encoding_none_encode;
        case JOLT_ENCODING_RLE: return jolt_encoding_rle_encode;
        case JOLT_ENCODING_JRLE: return jolt_encoding_jrle_encode;
        default: return NULL;
    }
}

jolt_encoding_t jolt_encoding_get_decoder( jolt_encoding_type_t encoding )
{
    switch( encoding ) {
        case JOLT_ENCODING_NONE: return jolt_encoding_none_decode;
        case JOLT_ENCODING_RLE: return jolt_encoding_rle_decode;
        case JOLT_ENCODING_JRLE: return jolt_encoding_jrle_decode;
        default: return NULL;
    }
}

const char *jolt_encoding_get_str( jolt_encoding_type_t encoding )
{
    switch( encoding ) {
        case JOLT_ENCODING_NONE: return "NONE";
        case JOLT_ENCODING_RLE: return "RLE";
        case JOLT_ENCODING_JRLE: return "JRLE";
        default: return "unknown";
    }
}
