#ifndef JOLT_SYSCORE_ENCODING_H__
#define JOLT_SYSCORE_ENCODING_H__

#include "stddef.h"
#include "stdint.h"
#include "string.h"

/**
 * @brief General signature of a nonstream encoder/decoder
 * @param[out] out Output buffer
 * @param[in] out_len length of output buffer.
 * @param[in] in RLE encoded data.
 * @return Minimum output buffer size to encode the data. If the minimum output
 * buffer size cannot be determined, return -1;
 */
typedef int ( *jolt_encoding_t )( uint8_t *, size_t, const uint8_t *, size_t );

/**
 * @brief Encoding method for copying/printing the display buffer.
 */
enum {
    JOLT_ENCODING_NONE = 0,
    JOLT_ENCODING_RLE,
    JOLT_ENCODING_JRLE,
};
typedef uint8_t jolt_encoding_type_t;

/**
 * @brief Get encoder for a given encoding type.
 * @param[in] encoding encoding val.
 * @return Encoder function.
 */
jolt_encoding_t jolt_encoding_get_encoder( jolt_encoding_type_t encoding );

/**
 * @brief Get decoder for a given encoding type.
 * @param[in] encoding encoding val.
 * @return Decoder function.
 */
jolt_encoding_t jolt_encoding_get_decoder( jolt_encoding_type_t encoding );

/**
 * @brief Get a human-readable string identifier for an encoding scheme.
 * @param[in] encoding encoding val.
 * @return string
 */
const char *jolt_encoding_get_str( jolt_encoding_type_t encoding );

/**
 * @brief Dummy encoding scheme that just copies `in` to `out`.
 */
int jolt_encoding_none_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

/**
 * @brief Dummy decoding scheme that just copies `in` to `out`.
 */
int jolt_encoding_none_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

/**
 * @brief Run-length encoder
 */
int jolt_encoding_rle_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

/**
 * @brief Run-length decoder
 */
int jolt_encoding_rle_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

/**
 * @brief Jolt run-length encoder (optimized for screen buffers)
 */
int jolt_encoding_jrle_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

/**
 * @brief Jolt run-length decoder (optimized for screen buffers)
 */
int jolt_encoding_jrle_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

/**
 * @brief ZLIB encoder
 */
int jolt_encoding_zlib_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

/**
 * @brief ZLIB encoder
 */
int jolt_encoding_zlib_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len );

#endif
