/**
 * Jolt Run-Length Encoding (JRLE)
 * JRLE encoding is based off of RLE, but significantly reduces overhead during
 * sequences of differing bytes. It is still incredibly fast, memory-constant,
 * and the exact required minimum decoded buffer can be computed without a
 * buffer. JRLE was developed to provide a quick, manageable way to perform
 * dispay buffer dumps of a 128x64 monochrome OLED screen.
 *
 * There are two sequence types: `repeating` and `differing`. Each sequence is
 * preceeded with a `uint8_t` counter.
 *     * Repeating counters have the MSb 1 (i.e. true == counter & 0x80)
 *     * Differing counters have the MSb 0 (i.e. false == counter & 0x80)
 * The lower 7 bits are used for counting, meaning a max interpretted counter
 * value of 0x7F (127).
 *
 * Edge-cases:
 *     * When a differing sequence is followed by a repeating sequence, the
 *       first byte of the repeating sequence is NOT to be including with the
 *       previous differing sequence.
 *       i.e. input "abcdd" gets encoded as `{3, 'a', 'b', 'c', 2 | 0x80, 'd'}`.
 *
 * The worst-case scenario for RLE is where every byte changes; this results in
 * an encoded output `2L`, where L is the input length.
 *
 * The worst-case scenario for JRLE is where every byte changes; this results in
 * an encoded output `L + ceil(L / 127)`.
 */

//#define LOG_LOCAL_LEVEL 4

#include "assert.h"
#include "esp_log.h"
#include "syscore/encoding.h"

#define OPCODE_MASK ( (uint8_t)0x80 )

#define REPEAT_MODE 1
#define DIFF_MODE   0

static const char TAG[] = "syscore/encoding/jrle";

int jolt_encoding_jrle_encode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
    uint8_t prev_c, curr_c, next_c;
    uint8_t n = 0;
    size_t counter_idx;
    size_t out_idx = 0;

    assert( in_len >= 3 );  // TODO handle edge case

#if 1
    prev_c = in[0];
    curr_c = in[1];
    next_c = in[2];

    if( prev_c == curr_c ) { n = OPCODE_MASK + 1; }
    else if( curr_c == next_c ) {
        /* First character is differing, followed by a repeating */
        ESP_LOGD( TAG, "(%d) Writing differing counter %d (0x%02X) to idx %d", __LINE__, 1, 1, out_idx );
        if( out && out_idx < out_len ) out[out_idx] = 1;
        out_idx++;
        ESP_LOGD( TAG, "(%d) Writing differing value \"%c\" (0x%02X) to idx %d", __LINE__, (char)prev_c, prev_c,
                  out_idx );
        if( out && out_idx < out_len ) out[out_idx] = prev_c;
        out_idx++;

        n = 0;
    }
    else {
        /* Ongoing Differing Sequence */
        n           = 1;
        counter_idx = 0;
        out_idx++;

        ESP_LOGD( TAG, "(%d) Writing differing value \"%c\" (0x%02X) to idx %d", __LINE__, (char)prev_c, prev_c,
                  out_idx );
        if( out && out_idx < out_len ) out[out_idx] = prev_c;
        out_idx++;
    }
#endif

    for( size_t in_idx = 1; in_idx < in_len; in_idx++ ) {
        prev_c = in[in_idx - 1];
        curr_c = in[in_idx];

        if( in_idx != in_len - 1 ) { next_c = in[in_idx + 1]; }

        if( 0 == n ) {
            if( in_idx == in_len - 1 ) {
                /* There is no next */
                if( out && out_idx < out_len ) out[out_idx] = 1;
                out_idx++;
                if( out && out_idx < out_len ) out[out_idx] = curr_c;
                out_idx++;
            }
            else if( curr_c == next_c ) {
                n = OPCODE_MASK + 1;
            }
            else {
                counter_idx = out_idx;
                out_idx++;
                ESP_LOGD( TAG, "(%d) Writing differing value \"%c\" (0x%02X) to idx %d", __LINE__, (char)curr_c,
                          curr_c, out_idx );
                if( out && out_idx < out_len ) out[out_idx] = curr_c;
                out_idx++;
                n++;
            }
            continue;
        }

        if( n & OPCODE_MASK ) {
            /* Repeat Sequence */
            if( prev_c == curr_c ) {
                n++;
                if( n == UINT8_MAX || in_idx == in_len - 1 ) {
                    ESP_LOGD( TAG, "(%d) Writing repeat counter %d (0x%02X) to idx %d", __LINE__, n & ~OPCODE_MASK, n,
                              out_idx );
                    if( out && out_idx < out_len ) out[out_idx] = n;
                    out_idx++;
                    ESP_LOGD( TAG, "(%d) Writing repeat value \"%c\" (0x%02X) to idx %d", __LINE__, (char)curr_c,
                              curr_c, out_idx );
                    if( out && out_idx < out_len ) out[out_idx] = curr_c;
                    out_idx++;
                    n = 0;
                }
            }
            else {
                /* Break in repeat sequence */
                ESP_LOGD( TAG, "(%d) Writing repeat counter %d (0x%02X) to idx %d", __LINE__, n & ~OPCODE_MASK, n,
                          out_idx );
                if( out && out_idx < out_len ) out[out_idx] = n;
                out_idx++;

                ESP_LOGD( TAG, "(%d) Writing repeat value \"%c\" (0x%02X) to idx %d", __LINE__, (char)prev_c, prev_c,
                          out_idx );
                if( out && out_idx < out_len ) out[out_idx] = prev_c;
                out_idx++;

                if( in_idx == in_len - 1 || curr_c != next_c ) {
                    counter_idx = out_idx;
                    out_idx++;
                    ESP_LOGD( TAG, "(%d) Writing differing value \"%c\" (0x%02X) to idx %d", __LINE__, (char)curr_c,
                              curr_c, out_idx );
                    if( out && out_idx < out_len ) out[out_idx] = curr_c;
                    out_idx++;
                    n = 1;
                    if( in_idx == in_len - 1 ) {
                        ESP_LOGD( TAG, "(%d) Writing differing counter %d (0x%02X) to idx %d", __LINE__,
                                  n & ~OPCODE_MASK, n, counter_idx );
                        if( out && counter_idx < out_len ) out[counter_idx] = n;
                        n = 0;
                    }
                }
                else {
                    n = OPCODE_MASK + 1;
                }
            }
        }
        else {
            /* Differing Sequence */
            if( prev_c == curr_c ) ESP_LOGE( TAG, "OOF" );
            if( in_idx == in_len - 1 || curr_c != next_c ) {
                n++;
                ESP_LOGD( TAG, "(%d) Writing differing value \"%c\" (0x%02X) to idx %d (Count=%d)", __LINE__,
                          (char)curr_c, curr_c, out_idx, n );
                if( out && out_idx < out_len ) out[out_idx] = curr_c;
                out_idx++;
                if( n == OPCODE_MASK - 1 || in_idx == in_len - 1 ) {
                    ESP_LOGD( TAG, "(%d) Writing differing counter %d (0x%02X) to idx %d", __LINE__, n & ~OPCODE_MASK,
                              n, counter_idx );
                    if( out && counter_idx < out_len ) out[counter_idx] = n;
                    n = 0;
                }
            }
            else {
                /* Break in differing sequence */
                ESP_LOGD( TAG, "(%d) Writing differing counter %d (0x%02X) to idx %d", __LINE__, n & ~OPCODE_MASK, n,
                          counter_idx );
                if( out && counter_idx < out_len ) out[counter_idx] = n;
                n = OPCODE_MASK + 1;
            }
        }
    }
    return out_idx;
}

int jolt_encoding_jrle_decode( uint8_t *out, size_t out_len, const uint8_t *in, size_t in_len )
{
    size_t out_idx = 0;

    if( 0 == in_len || NULL == in ) {
        ESP_LOGE( TAG, "Param Error" );
        return -1;
    }

    for( size_t in_idx = 0; in_idx < in_len; ) {
        uint8_t n = in[in_idx];
        ESP_LOGD( TAG, "Parsed Count %d (%s) at idx %d.", n & ~OPCODE_MASK,
                  n & OPCODE_MASK ? "repeating" : "differing", in_idx );
        in_idx++;
        if( in_idx >= in_len ) {
            ESP_LOGE( TAG, "(%d) Decoding Error.", __LINE__ );
            return -1;
        }

        if( n & OPCODE_MASK ) {
            n = n & ~OPCODE_MASK;

            for( uint8_t i = 0; i < n; i++ ) {
                ESP_LOGD( TAG, "(%d) Decoding repeating \"%c\" (0x%02X) to %d", __LINE__, in[in_idx], in[in_idx],
                          out_idx );
                if( out && out_idx < out_len ) out[out_idx] = in[in_idx];
                out_idx++;
            }
            in_idx++;
        }
        else {
            for( uint8_t i = 0; i < n; i++ ) {
                if( in_idx >= in_len ) {
                    ESP_LOGE( TAG, "(%d) Decoding Error.", __LINE__ );
                    return -1;
                }

                ESP_LOGD( TAG, "(%d) Decoding differing \"%c\" (0x%02X) to %d", __LINE__, in[in_idx], in[in_idx],
                          out_idx );
                if( out && out_idx < out_len ) out[out_idx] = in[in_idx];
                out_idx++;
                in_idx++;
            }
        }
    }
    return out_idx;
}
