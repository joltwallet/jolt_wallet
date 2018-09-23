//         ATMEL Microcontroller Software Support  -  Colorado Springs, CO -
// ----------------------------------------------------------------------------
// DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
// DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
// OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// ----------------------------------------------------------------------------

/** \file
 *  \brief  AES132 Helper Functions
 *  \author Tuwuh Sarwoprasojo, Atmel Bali Team
 *  \date   June 12, 2013
 */

#include <stdint.h>
#include <string.h>
#include "aes132_mac.h"
#include "aes132_comm_marshaling.h"
#include "mbedtls/aes.h"
#include "esp_log.h"
#include "sodium.h"

//---------------------------
// Static function prototype
//---------------------------
static uint8_t aes132crypt_generate_encrypt(struct aes132crypt_in_out *param);
static uint8_t aes132crypt_decrypt_verify(struct aes132crypt_in_out *param);
static uint8_t aes132crypt_nonce(struct aes132crypt_nonce_in_out *param);
static void aes132crypt_aes_engine_encrypt(uint8_t *data, uint8_t *key, bool init_flag);
static void aes132crypt_cbc_block(struct aes132crypt_in_out *param, uint8_t *output);
static void aes132crypt_ctr_block(struct aes132crypt_ctr_block_in_out *param);

/* Global Variables */
static const char TAG[] = "aes132_mac";

//----------------------
// Function definitions
//----------------------
//
/*
 * 128-bit (16-byte) input cleartext and output encrypted data are passed using pointer "data", and 128-bit (16-byte) AES key are passed using pointer "key".
 * This AES wrapper function is the building block for cryptographic calculation functions.
 * 
 * \param [in,out] data"  Pointer to 16-byte input and output data.
 */
static void aes132crypt_aes_engine_encrypt(uint8_t *data, uint8_t *key, 
        bool init_flag) {
    static mbedtls_aes_context aes;
    
    if ( true == init_flag  && NULL != key) {
        // Call AES context initialization (RoundKey calculation) here
        mbedtls_aes_setkey_enc( &aes, key, 128 );
    } else if ( NULL != data ) {
        mbedtls_aes_crypt_ecb( &aes, MBEDTLS_AES_ENCRYPT, data, data);
    } else {
        ESP_LOGE(TAG, "Invalid parameters to aes132crypt_aes_engine_encrypt");
    }
}


/** \brief This function does the Generation-Encryption Process as described in NIST Special Publication 800-38C, section 6.1.
 *
 * The input data to this function are "nonce", "payload", and "associated_data".
 * These inputs are processed using AES-CCM with the key passed in parameter "key".
 * The outputs are authentication value "auth_value" and encrypted data "ciphertext".
 * 
 * This function is used in majority of ATAES132 commands which uses "MAC Generation" and/or "Data Encryption".
 * For commands that do not encrypt data (for example Auth), "payload" can be set to NULL and "plen_bytes" set to 0.
 * The InMac or OutMac for ATAES132 commands is returned in "auth_value", while the encrypted InData or OutData is returned in "ciphertext".
 * 
 * Example for Auth command, Outbound, with optional authenticate field SerialNum[0:7] :
 * - "nonce" contains concatenation of 12-byte Nonce and 1-byte MacCount
 * - "payload" is set to NULL
 * - "plen_bytes" is set to 0
 * - "associated_data" contains 30-byte authenticate-only data specified in ATAES132 datasheet (Section I.6 in 8760A-CRYPTO-5/11).
 *   For this example, this is concatenation of:
 *   2-byte ManufacturingID, 1-byte Opcode (0x03), 1-byte Mode, 2-byte Param1, 2-byte Param2, 1-byte MacFlag, 4-byte 0x00, 1-byte 0x00,
 *   4-byte 0x00, 8-byte SerialNum[0:7], 4-byte 0x00.
 * - "alen_bytes" is set to 30
 * - "key" contains key used in the computation
 * - The 16-byte OutMac will be returned in "auth_value"
 * 
 * Example for EncWrite command with 16 bytes data:
 * - "nonce" contains concatenation of 12-byte Nonce and 1-byte MacCount
 * - "payload" contains the cleartext data to be written
 * - "plen_bytes" is set to 16
 * - "associated_data" contains 14-byte authenticate-only data specified in ATAES132 datasheet (Section I.20 in 8760A-CRYPTO-5/11).
 *   For this example, this is concatenation of:
 *   2-byte ManufacturingID, 1-byte Opcode (0x05), 1-byte Mode, 2-byte Param1, 2-byte Param2, 1-byte MacFlag, 5-byte 0x00.
 * - "alen_bytes" is set to 14
 * - "key" contains key used in the computation
 * - The 16-byte InMac will be returned in "auth_value", and 16-byte InData will be returned in "ciphertext"
 *
 * \param [in,out] param Pointer to structure for input/output parameters. Refer to aes132crypt_in_out.
 * \return status of the operation.
 */
uint8_t aes132crypt_generate_encrypt(struct aes132crypt_in_out *param) {
    // Local variables
    uint8_t cleartext_mac[16];
    struct aes132crypt_ctr_block_in_out ctr_param;
    
    // Initialize AES engine
    aes132crypt_aes_engine_encrypt(NULL, param->key, true);
    
    // Perform CBC blocks
    aes132crypt_cbc_block(param, cleartext_mac);

    // Perform CTR blocks
    ctr_param.auth_value_in  = cleartext_mac;
    ctr_param.data_in        = param->payload;
    ctr_param.plen_bytes     = param->plen_bytes; 
    ctr_param.nonce          = param->nonce;
    ctr_param.key            = param->key;
    ctr_param.auth_value_out = param->auth_value;
    ctr_param.data_out       = param->ciphertext;
    aes132crypt_ctr_block(&ctr_param);
    
    return AES132_FUNCTION_RETCODE_SUCCESS;
}


/** \brief This function does the Decryption-Verification Process as described in NIST Special Publication 800-38C, section 6.1.
 *
 * The input data to this function are "nonce", "auth_value", "ciphertext", and "associated_data".
 * These inputs are processed using AES-CCM with the key passed in parameter "key".
 * The output is the decrypted cleartext data, "payload".
 * 
 * This function is used in ATAES132 commands which uses "MAC Generation" and "Data Decryption".
 * The InMac or OutMac for ATAES132 commands is passed in "auth_value", while the decrypted InData or OutData is returned in "payload".
 * 
 * Example for EncRead command with 32 bytes data,
 * - "nonce" contains concatenation of 12-byte Nonce and 1-byte MacCount
 * - "auth_value" contains OutMac from the EncRead command
 * - "ciphertext" contains OutData from the EncRead command
 * - "plen_bytes" is set to 32 (32 bytes of data)
 * - "associated_data" contains 14-byte authenticate-only data specified in ATAES132 datasheet (Section I.16 in 8760A-CRYPTO-5/11).
 *   For this example, this is concatenation of:
 *   2-byte ManufacturingID, 1-byte Opcode (0x04), 1-byte Mode, 2-byte Param1, 2-byte Param2, 1-byte MacFlag, 5-byte 0x00.
 * - "alen_bytes" is set to 14
 * - "key" contains key used in the computation
 * - The cleartext data will be returned in "payload"
 *
 * \param [in,out] param Pointer to structure for input/output parameters. Refer to aes132crypt_in_out.
 * \return status of the operation.
 */
uint8_t aes132crypt_decrypt_verify(struct aes132crypt_in_out *param) {
    // Local variables
    uint8_t computed_mac[16] = { 0 };
    uint8_t provided_mac[16] = { 0 };
    struct aes132crypt_ctr_block_in_out ctr_param;

    // Initialize AES engine
    aes132crypt_aes_engine_encrypt(NULL, param->key, true);
    
    // Perform CTR blocks
    ctr_param.auth_value_in  = param->auth_value;
    ctr_param.data_in        = param->ciphertext;
    ctr_param.plen_bytes     = param->plen_bytes; 
    ctr_param.nonce          = param->nonce;
    ctr_param.key            = param->key;
    ctr_param.auth_value_out = provided_mac;
    ctr_param.data_out       = param->payload;
    aes132crypt_ctr_block(&ctr_param);
    
    // Perform CBC blocks
    aes132crypt_cbc_block(param, computed_mac);
    
    // Verify the cleartext MAC T against calculated
    /* todo: prevent side-channel attacks */
    if (memcmp(provided_mac, computed_mac, 16) == 0) {
        return AES132_FUNCTION_RETCODE_SUCCESS;
    } else {
        // As mandated by NIST Special Publication 800-38C, section 6.2,
        //   if verification fails, the payload shall not be revealed.
        // We reset the payload to all 0's
        ESP_LOGI(TAG, "computeMAC "
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                computed_mac[0], computed_mac[1], computed_mac[2],
                computed_mac[3], computed_mac[4], computed_mac[5],
                computed_mac[6], computed_mac[7], computed_mac[8],
                computed_mac[9], computed_mac[10], computed_mac[11],
                computed_mac[12], computed_mac[13], computed_mac[14],
                computed_mac[15]);

        sodium_memzero(param->payload, param->plen_bytes);
        return AES132_DEVICE_RETCODE_MAC_ERROR;
    }
}


/** \brief This function does one pass of Matyas-Meyer-Oseas compression function.
 *
 * The function can be used to generate random nonce for Nonce and NonceCompute commands in ATAES132.
 * These are described in ATAES132 datasheet (Appendix I.31 and I.32 in 8760A-CRYPTO-5/11).
 * Please refer to ATAES132 datasheet for data_in (block A in datasheet) and key (block B in datasheet) formatting.
 *
 * All inputs are 16-byte, and output is also 16-byte.
 * To be used as Nonce for ATAES132 device, the output must be truncated to the first 12 bytes.
 *
 * \param [in,out] param Pointer to structure for input/output parameters. Refer to aes132crypt_nonce_in_out.
 * \return status of the operation.
 */
uint8_t aes132crypt_nonce(struct aes132crypt_nonce_in_out *param) {
    // Initialize AES engine
    aes132crypt_aes_engine_encrypt(NULL, param->key, true);
    
    // Run AES for input data
    memcpy(param->data_out, param->data_in, 16);
    aes132crypt_aes_engine_encrypt(param->data_out, param->key, false);
    
    // XOR the result with input data
    for (uint8_t i = 0; i < 16; i++) {
        param->data_out[i] ^= param->data_in[i];
    }
    
    return AES132_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function implements CBC blocks for AES-CCM computation used by ATAES132 device.
 *
 * The formatting function for input data is included in this function.
 *
 * \param [in] param  Pointer to structure for input/output parameters. Refer to aes132crypt_in_out.
 * \param [out] output  Pointer to 16-byte MAC (called T in NIST Special Publication 800-38C, or "cleartext MAC" in ATAES132 datasheet).
 */
static void aes132crypt_cbc_block(struct aes132crypt_in_out *param, uint8_t *output)
{
    // Local variables
    uint8_t temporary[16];
    uint8_t *p_input;

    uint8_t alen_remaining;
    uint8_t plen_remaining;
    uint8_t pass;
    uint8_t i;

    //-----------------------------
    // CBC block
    // output[] (provided by caller) is used to hold Y_n
    // temporary[] is used to hold B_n
    //-----------------------------
    // Initialize values
    memset(output, 0x00, 16);
    alen_remaining = param->alen_bytes;
    plen_remaining = param->plen_bytes;
    pass = 0;
    
    // To save memory space, the CBC mode operations (block B) are done serially
    //   pass is used to indicate the index of B (B_pass)
    do {
        // Input is initialized to point to temporary array for filling
        p_input = temporary;
    
        if (pass == 0) {
            // First block B_0
            // See table 2 app A.2.1 NIST SP800-38C
            // Byte 0: Flag
            // The flag consists of:
            //   Bit 7    Reserved = 0
            //       6    Adata, = 1 if there is associated data, = 0 otherwise
            //       5:3  Encoding of tag length  t: (t-2)/2. For AES132 t=16, (t-2)/2= 7
            //       2:0  Encoding of length of Q (encoding of payload length)  q: (q-1). For AES132 q=2, (q-1)= 1
            if (param->alen_bytes > 0) {
                *p_input++ = (1 << 6) | (7 << 3) | (1); // 0b0111 1001; 0x79
            } else {
                *p_input++ =            (7 << 3) | (1); // 0b0011 1001; 0x39 wtf nvm this never executes
            }
            // Byte 1 to 15-2 (13): N (13-byte nonce)
            memcpy(p_input, param->nonce, 13);
            p_input += 13;
            // Byte 13 to 15: Q (message/payload length)
            // For AES132, Q <= 32, so MSByte always zero
            *p_input++ = 0x00;
            *p_input++ = param->plen_bytes;
            // good so far
            
            // Point input back to the temporary array
            p_input = temporary;
        } else if (alen_remaining > 0) {
            if (pass == 1) {
                // Second block B_1
                // Byte 0 to 1: encoding of a (length of associated data)
                // For AES132, a <= 30, so MSByte always zero. Max 255 bytes of associated data.
                *p_input++ = 0x00;
                *p_input++ = param->alen_bytes;
                // Byte 2 to 15: first 14 bytes of associated data
                if (alen_remaining < 14) {
                    memcpy(p_input, param->associated_data, alen_remaining);
                    p_input += alen_remaining;
                    // If associated data is 14 bytes or less, pad with 0x00's
                    memset(p_input, 0x00, (14 - alen_remaining));
                    alen_remaining = 0;
                } else {
                    memcpy(p_input, param->associated_data, 14);
                    alen_remaining -= 14;
                }
                
                // Point input back to the temporary array
                p_input = temporary;
            } else {
                // Next blocks (B_2 ... B_u)
                // Byte 0 to 15: next 16 bytes of associated data
                if (alen_remaining < 16) {
                    memcpy(p_input, &param->associated_data[(param->alen_bytes)-alen_remaining], alen_remaining);
                    p_input += alen_remaining;
                    // If associated data is less than 16 bytes, pad with 0x00's
                    memset(p_input, 0x00, (16 - alen_remaining));
                    
                    // Point input back to the temporary array
                    p_input = temporary;
                    
                    alen_remaining = 0;                 
                } else {
                    // Point input directly to the data
                    p_input = &param->associated_data[(param->alen_bytes)-alen_remaining];
                    
                    alen_remaining -= 16;
                }
            }
        } else if (plen_remaining > 0) {
            // Next blocks (B_u+1 ... B_r)
            // Byte 0 to 15: next 16 bytes of payload
            if (plen_remaining < 16) {
                memcpy(p_input, &param->payload[(param->plen_bytes)-plen_remaining], plen_remaining);
                p_input += plen_remaining;
                // If payload is less than 16 bytes, pad with 0x00's
                memset(p_input, 0x00, (16 - plen_remaining));
                
                // Point input back to the temporary array
                p_input = temporary;
                
                plen_remaining = 0;
            } else {
                // Point input directly to the data
                p_input = &param->payload[(param->plen_bytes)-plen_remaining];
                
                plen_remaining -= 16;
            }
        }
        
        // XOR B_i with Y_i and put in Y_i
        for (i = 0; i < 16; i++) {
            output[i] ^= *p_input++;
        }
        // Run AES
        aes132crypt_aes_engine_encrypt(output, param->key, false);
        // Y_i is now in array output
        
        // Increment pass
        pass++;
    } while ((alen_remaining > 0) || (plen_remaining > 0));
    // T (Y_r) is now in output[]
    // Because t is fixed to 16, T = MSB_t_bytes(Y_r) = Y_r 
}


/** \brief This function implements CTR blocks for AES-CCM computation used by ATAES132 device.
 *
 * The formatting function for counter blocks is included in this function.
 * The encryption and decryption process is the same using CTR.
 * For encryption, auth_value_in and data_in are set to cleartext MAC (T) and payload (cleartext data), respectively.
 * The encrypted MAC and ciphertext are returned in auth_value_out and data_out, respectively.
 * For decryption, auth_value_in and data_in are set to encrypted MAC and ciphertext, respectively.
 * The cleartext MAC and payload (cleartext data) are returned in auth_value_out and data_out, respectively.
 *
 * \param [in, out] param  Pointer to structure for input/output parameters. Refer to aes132crypt_ctr_block_in_out.
 */
static void aes132crypt_ctr_block(struct aes132crypt_ctr_block_in_out *param)
{
    // Local variables
    uint8_t *p_ctr;
    uint8_t *p_data_in;
    uint8_t *p_data_out;
    uint8_t plen_remaining;
    uint8_t ctr_enc[16];
    uint8_t ctr[16];
    uint8_t i;
    
    //-----------------------------
    // Counter blocks
    //----------------------------- 
    // Prepare first counter block Ctr_0
    // ctr[] is used to hold Ctr_j
    p_ctr = ctr;
    // Byte 0: flag, bit 7:3 = 0, bit 2:0 = (q-1) = 1
    *p_ctr++ = (1);
    // Byte 1 to 13: N (13-byte nonce)
    memcpy(p_ctr, param->nonce, 13);
    p_ctr += 13;
    // Byte 14 to 15: i (counter)
    *p_ctr++ = 0x00;
    *p_ctr++ = 0x00;
    
    // Point to first payload
    p_data_in = param->data_in;
    p_data_out = param->data_out;   
    plen_remaining = param->plen_bytes;
    
    do {
        // Copy Ctr_j to S_j for encryption
        memcpy(ctr_enc, ctr, 16);
        
        // Run AES
        aes132crypt_aes_engine_encrypt(ctr_enc, param->key, false);
        // S_j is now in array ctr_enc[]

        // If counter byte = 0, it is the first pass, encrypt/decrypt auth_value
        // Otherwise, it is second and subsequent pass, encrypt/decrypt payload
        if (ctr[15] == 0) {
            // XOR input auth value with S_j and put in output auth value
            for (i = 0; i < 16; i++) {
                param->auth_value_out[i] = param->auth_value_in[i] ^ ctr_enc[i];
            }
        } else {
            // XOR input data with S_j and put in output
            if (plen_remaining < 16) {
                // Encrypt/decrypt only the remaining bytes
                for (i = 0; i < plen_remaining; i++) {
                    *p_data_out++ = *p_data_in++ ^ ctr_enc[i];
                }
                // Pad with 0x00's to fill 16-byte block
                for (; i < 16; i++) {
                    *p_data_out++ = 0x00 ^ ctr_enc[i];
                }
                plen_remaining = 0;
            } else {
                // Encrypt/decrypt 16 bytes (one block)
                for (i = 0; i < 16; i++) {
                    *p_data_out++ = *p_data_in++ ^ ctr_enc[i];
                }
                plen_remaining -= 16;
            }
        }
        
        // Increment the counter. Counter is in block Ctr byte 14 and 15.
        // We only use the LSByte, so payload is max 255 blocks 
        ctr[15]++;
    } while (plen_remaining > 0);   
}


/** \brief This function generates or store a 12-byte nonce for use by the helper functions.
 * 
 * Upon successful execution, the "nonce" struct is updated by this function:
 * - The value (nonce[0] to nonce[11]) is filled by the nonce
 * - MacCount (nonce[12]) is reset to zero
 * - Valid flag is set
 * The two modes are supported, random nonce and inbound nonce, determined by mode parameter.
 * If using inbound nonce, "random" parameter is not used and can take NULL value.
 *
 * \param [in,out] param Pointer to structure for input/output parameters. Refer to aes132h_nonce_in_out.
 * \return status of the operation.
 */
uint8_t aes132h_nonce(struct aes132h_nonce_in_out *param)
{
    uint8_t ret_code;
    struct aes132crypt_nonce_in_out aes132crypt_nonce_param;
    uint8_t block_a[16];
    uint8_t block_b[16];
    uint8_t data_out[16];
    
    // Check parameters
    if ( (param->mode & ~0x03) != 0 ) {
        ESP_LOGE(TAG, "aes132h_nonce; bad param; "
                "mode<7-2> must be 0");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;   
    }
    else if (!param->random && ((param->mode & 0x01) != 0)) {
        ESP_LOGE(TAG, "aes132h_nonce; bad param; "
                "Random Pointer and Mode Disagree");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;   
    }
    else if (!param->in_seed) {
        ESP_LOGE(TAG, "aes132h_nonce; bad param; "
                "");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;   
    }
    else if (!param->nonce) {
        ESP_LOGE(TAG, "aes132h_nonce; bad param; "
                "No aes132h_nonce_s structure specified.");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;   
    }
    
    // Check mode parameter bit 0 (Random/Inbound) and do appropriate operations
    if (param->mode & 0x01) {
        // Assemble block A and B
        block_a[0] = AES132_OPCODE_NONCE;
        block_a[1] = param->mode;
        block_a[2] = 0x00;
        block_a[3] = 0x00;
        memcpy(&block_a[4], param->in_seed, 12);
        block_b[0] = AES132_MANUFACTURING_ID >> 8;
        block_b[1] = AES132_MANUFACTURING_ID & 0xFF;
        block_b[2] = 0x00;
        block_b[3] = 0x00;
        memcpy(&block_b[4], param->random, 12);
        
        // Execute cryptographic algorithm
        aes132crypt_nonce_param.data_in = block_a;
        aes132crypt_nonce_param.key = block_b;
        aes132crypt_nonce_param.data_out = data_out;
        ret_code = aes132crypt_nonce(&aes132crypt_nonce_param);
        if (ret_code != AES132_FUNCTION_RETCODE_SUCCESS)
            return ret_code;
            
        // Copy the first 12-byte as a nonce value
        memcpy(param->nonce->value, data_out, 12);

        // Set the random flag to true (Random nonce)
        param->nonce->random = true;
    } else {
        // Copy the InSeed as a nonce value
        memcpy(param->nonce->value, param->in_seed, 12);
        
        // Set the random flag to FALSE (Inbound nonce)
        param->nonce->random = false;
    }
    
    // Reset the MacCount, set the valid flag
    param->nonce->value[12] = 0x00;
    param->nonce->valid = true;
    
    return AES132_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function converts 4-byte CountValue output from Counter command to integer counts.
 * 
 * "count_integer" will take value between 0 to 2097151.
 * Bad parameter error is returned if count_value has illegal value.
 *
 * \param [in] count_value  Pointer to 4-byte CountValue.
 * \param [out] count_integer  Pointer to 32-bit integer representing current value of the counter.
 * \return status of the operation.
 */
uint8_t aes132h_decode_count_value(uint8_t *count_value, uint32_t *count_integer)
{
    uint8_t lin_count;
    uint8_t count_flag;
    uint16_t bin_count;

    uint8_t int_lin_count;
    
    // Check parameters
    if (!count_value || !count_integer)
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    
    // Extract LinCount, CountFlag, and BinCount from CountValue byte 0
    lin_count = count_value[0];
    count_flag = count_value[1];
    bin_count = (count_value[2] << 8) + count_value[3];

    // Check CountFlag and LinCount validity
    // CountFlag can only take values 0, 2, 4, 6. So, check by masking with 00000110b.
    // 2's complement of LinCount must be power of 2. Check if it's the case, based on trick from
    //    http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    if (    ((count_flag & ~(0x06)) != 0)
            || (lin_count == 0)
            || ((((uint8_t) ~lin_count) + 1) & ((uint8_t) ~lin_count))
        )
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    
    // Convert LinCount to integer
    int_lin_count = 7;
    while (lin_count <<= 1) {
        int_lin_count--;
    }
    
    // Calculate the result, which is: BinCount*32 + (CountFlag/2)*8 + integer(LinCount)
    *count_integer = (bin_count << 5) + (count_flag << 2) + int_lin_count;
    
    return AES132_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function converts integer counts to a 8-byte value to be written to Counter field in ATAES132 configuration memory.
 *
 * "count_integer" input is maximum 2097151.
 * Bad parameter error is returned if count_integer takes value higher than this maximum.
 * User must provide enough space for "counter".
 *
 * \param [in] count_integer  32-bit integer representing the desired value of the counter.
 * \param [out] counter_field  Pointer to 8-byte Counter field to be written to Configuration Area.
 * \return status of the operation. 
 */
uint8_t aes132h_encode_counter_field(uint32_t count_integer, uint8_t *counter_field)
{
    uint8_t lin_count;
    uint8_t count_flag;
    uint16_t bin_count;
    
    // Check parameters
    if ((!counter_field) || (count_integer > AES132_COUNTER_MAX))
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    
    // Extract BinCount, CountFlag, and LinCount from intended integer counter
    bin_count = count_integer >> 5; // count_integer / 32;
    count_integer %= 32;
    
    count_flag = (count_integer >> 3) << 1; // (count_integer / 8) << 1;
    count_integer %= 8;
    
    lin_count = 0xFF << count_integer;
    
    // Reset all LinCount to zero first
    counter_field[0] = 0x00;
    counter_field[1] = 0x00;
    counter_field[2] = 0x00;
    counter_field[3] = 0x00;
    
    // Put BinCount on both BinCountA and BinCountB
    counter_field[4] = bin_count & 0xFF;
    counter_field[5] = bin_count >> 8;
    counter_field[6] = bin_count & 0xFF;
    counter_field[7] = bin_count >> 8;
    
    // Update the fields depending on CountFlag
    switch (count_flag) {
        case 0:
            // LSB of LinCountA = LinCount, MSB of LinCountA = 0
            // LinCountB = 0        
            // BinCountB = BinCount - 1, clip to 0
            // BinCountA = BinCount
            counter_field[0] = lin_count;
            counter_field[1] = 0xFF;
            if (bin_count != 0) {
                counter_field[4] = ((bin_count - 1) & 0xFF);
                counter_field[5] = ((bin_count - 1) >> 8);
            }
        break;
        case 2:
            // LSB of LinCountA = 0, MSB of LinCountA = LinCount
            // LinCountB = 0            
            // BinCountB = BinCount - 1, clip to 0
            // BinCountA = BinCount
            counter_field[1] = lin_count;           
            if (bin_count != 0) {
                counter_field[4] = ((bin_count - 1) & 0xFF);
                counter_field[5] = ((bin_count - 1) >> 8);
            }
        break;
        case 4:
            // LinCountA = 0
            // LSB of LinCountB = LinCount, MSB of LinCountB = 0xFF         
            // BinCountB = BinCount, BinCountA = BinCount
            counter_field[2] = lin_count;
            counter_field[3] = 0xFF;            
        break;
        case 6:
            // LinCountA = 0
            // LSB of LinCountB = LinCount, MSB of LinCountB = 0            
            // BinCountB = BinCount, BinCountA = BinCount
            counter_field[3] = lin_count;           
        break;
    }
    
    return AES132_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function assembles FirstBlock and SecondBlock for use in ATAES132 host commands.
 *
 * It is used when application uses second ATAES132 for host functionality, instead of the helper functions.
 * The blocks are assembled according to input parameters, which is passed using "aes132h_build_auth_block_in_out" struct. All unused pointer parameters can take NULL value.
 * In addition, user must also provide a random flag, which indicates whether the client (first ATAES132 device) uses random or inbound nonce when running its command.
 * 
 * Host commands supported:
 * - AuthCheck, first_block is 11 bytes
 *   For use with commands: Auth Outbound, Auth Mutual, Counter Read
 * - AuthCompute, first_block is 11 bytes
 *   For use with commands: Auth Inbound, Auth Mutual, Counter Increment
 * - DecRead, first_block is 6 bytes
 *   For use with command: EncRead
 * - KeyExport, first_block is 6 bytes
 *   For use with command: KeyLoad
 * - KeyImport, first_block is 6 bytes
 *   For use with commands: KeyCreate, KeyExport (Mode:0 = 0)
 * - WriteCompute, first_block is 6 bytes
 *   For use with command: EncWrite
 * 
 * first_block is always built, regardless of mode parameter.
 * second_block is built if any one of optional authenticate fields are selected in mode bit 7-5 (usage_counter, serial_num, small_zone). second_block is 16 bytes for all commands.
 * 
 * User must provide enough space for first_block (always used) and second_block (if used).
 *
 * \param [in,out] param Pointer to structure for input/output parameters. Refer to aes132h_build_auth_block_in_out.
 * \return status of the operation. 
 */
uint8_t aes132h_build_auth_block(struct aes132h_build_auth_block_in_out *param)
{
    uint8_t *p_first_block;
    
    // Check parameters
    if (    (!param->first_block)
            || ((param->mode & 0x20) && !param->usage_counter)
            || ((param->mode & 0x40) && !param->serial_num)
            || ((param->mode & 0x80) && !param->small_zone)
            || ((param->mode & 0xE0) && !param->second_block)
            || ((param->opcode == AES132_OPCODE_COUNTER) && !param->count_value)
        )
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    
    // Fill in the FirstBlock
    p_first_block = param->first_block;
    
    // First byte is the Opcode for AuthCompute and AuthCheck, Mode for other commands
    if (    (param->host_opcode == AES132_OPCODE_AUTH_COMPUTE)
            || (param->host_opcode == AES132_OPCODE_AUTH_CHECK)) {
        *p_first_block++ = param->opcode;
    }
    
    *p_first_block++ = param->mode;
    *p_first_block++ = param->param1 >> 8;
    *p_first_block++ = param->param1 & 0xFF;
    *p_first_block++ = param->param2 >> 8;
    *p_first_block++ = param->param2 & 0xFF;
    
    // Check whether the MAC is used as input or output
    if (    (param->host_opcode == AES132_OPCODE_AUTH_COMPUTE)
            || (param->host_opcode == AES132_OPCODE_WRITE_COMPUTE)
            || (param->host_opcode == AES132_OPCODE_KEY_CREATE)) {
        // MAC used as input to device, Input bit (bit 1) always 1
        *p_first_block++ = (param->random)?
                           (AES132_MAC_FLAG_RANDOM | AES132_MAC_FLAG_INPUT):
                           AES132_MAC_FLAG_INPUT;
    } else {
        // MAC is output from device, Input bit (bit 1) always 0
        *p_first_block++ = (param->random)? AES132_MAC_FLAG_RANDOM: 0x00;
    }
    
    // For Counter command, fill with the CountValue
    if (    (param->host_opcode == AES132_OPCODE_AUTH_COMPUTE)
            || (param->host_opcode == AES132_OPCODE_AUTH_CHECK)) {
        if (param->opcode == AES132_OPCODE_COUNTER) {
            memcpy(p_first_block, param->count_value, 4);
        } else {
            memset(p_first_block, 0x00, 4);
        }
    }

    // Append additional data to SecondBlock if specified by mode parameter
    if (param->mode & 0xE0) {
        if (param->mode & 0x20) {
            memcpy(&param->second_block[0], param->usage_counter, 4);
        } else {
            memset(&param->second_block[0], 0x00, 4);
        }
        
        if (param->mode & 0x40) {
            memcpy(&param->second_block[4], param->serial_num, 8);
        } else {
            memset(&param->second_block[4], 0x00, 8);
        }
        
        if (param->mode & 0x80) {
            memcpy(&param->second_block[12], param->small_zone, 4);
        } else {
            memset(&param->second_block[12], 0x00, 4);
        }
    }
    
    return AES132_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function generates authentication/integrity MAC and encrypts data for use in ATAES132 commands.
 *
 * Operations supported by this function:
 * - Generate InMac for Auth, Inbound-only (Mode[1:0] = 01b)
 * - Generate InMac for Auth, Mutual (Mode[1:0] = 11b)
 * - Generate InMac for Counter increment (Mode[1:0] = 10b)
 * - Encrypt InData and generate InMac for EncWrite
 * - Encrypt InData and generate InMac for Decrypt
 * - Encrypt InData and generate InMac for KeyLoad
 * - Generate InMac for Lock ZoneConfig[Zone].ReadOnly (Mode[1:0] = 11b)
 * 
 * All unused pointer in parameter struct can take NULL value.
 * The use of second authentication blocks (usage_counter, serial_num, small_zone) is determined by mode bit 7-5.
 * count_value is used only for Counter command, and not used for other commands.
 * in_mac is never used for this function.
 * 
 * The "nonce" struct has to be passed with valid member set to true, otherwise the function returns with error.
 * The "nonce" struct is updated by this function:
 * - If MacCount has reached maximum value (255), valid flag is cleared
 * - MacCount (nonce[12]) is incremented
 * 
 * The function returns with bad parameter error if:
 * - Passed with opcode/mode combination that is not supported
 * - At least one of used pointer parameters has NULL value
 * 
 * User must provide enough space for output parameters (out_mac and out_data).
 *
 * \param [in,out] param Pointer to structure for input/output parameters. Refer to aes132h_in_out.
 * \return status of the operation. 
 */
uint8_t aes132h_mac_compute_encrypt(struct aes132h_in_out *param)
{
    uint8_t ret_code;
    struct aes132crypt_in_out aes132crypt_param;    
    uint8_t associated_data[30];
    
    // Check parameters
    if (    (!param->key) || (!param->nonce) || (!param->out_mac)
            || ((param->mode & 0x20) && !param->usage_counter)
            || ((param->mode & 0x40) && !param->serial_num)
            || ((param->mode & 0x80) && !param->small_zone)
            || ((param->opcode == AES132_OPCODE_COUNTER) && !param->count_value)
            || (((param->opcode == AES132_OPCODE_ENC_WRITE)
                || (param->opcode == AES132_OPCODE_DECRYPT)
                || (param->opcode == AES132_OPCODE_KEY_LOAD)) && (!param->in_data || !param->out_data))
        )
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    
    // Check nonce validity
    if (param->nonce->valid == false)
        return AES132_DEVICE_RETCODE_NONCE_ERROR;
    
    // Arrange associated_data (see ATAES132 datasheet appendix I)
    associated_data[0] = AES132_MANUFACTURING_ID >> 8;
    associated_data[1] = AES132_MANUFACTURING_ID & 0xFF;
    associated_data[2] = param->opcode;
    associated_data[3] = param->mode;
    associated_data[4] = param->param1 >> 8;
    associated_data[5] = param->param1 & 0xFF;
    associated_data[6] = param->param2 >> 8;
    associated_data[7] = param->param2 & 0xFF;
    associated_data[8] = (param->nonce->random)?
                         (AES132_MAC_FLAG_RANDOM | AES132_MAC_FLAG_INPUT):
                         AES132_MAC_FLAG_INPUT;   // Input bit (bit 1) always 1
                         
    if (param->opcode == AES132_OPCODE_COUNTER) {
        memcpy(&associated_data[9], param->count_value, 4);
    } else {
        memset(&associated_data[9], 0x00, 4);
    }
    associated_data[13] = 0x00;
    
    // Append additional associated_data if specified by mode parameter
    if (param->mode & 0x20) {
        memcpy(&associated_data[14], param->usage_counter, 4);
    } else {
        memset(&associated_data[14], 0x00, 4);
    }
    if (param->mode & 0x40) {
        memcpy(&associated_data[18], param->serial_num, 8);
    } else {
        memset(&associated_data[18], 0x00, 8);
    }
    if (param->mode & 0x80) {
        memcpy(&associated_data[26], param->small_zone, 4);
    } else {
        memset(&associated_data[26], 0x00, 4);
    }
    
    // Check if Client Decryption mode, set MacCount to EMacCount, adjust associated_data to suit 
    if ((param->opcode == AES132_OPCODE_DECRYPT) && (param->param2 & 0xFF00)) {
        param->nonce->value[12] = (uint8_t) (param->param2 >> 8);
        associated_data[2] = AES132_OPCODE_ENCRYPT;   // Use Encrypt opcode
        associated_data[4] = 0x00;                    // Upper byte of Param1 is 0
        associated_data[6] = 0x00;                    // Upper byte of Param2 is 0
        associated_data[8] &= ~AES132_MAC_FLAG_INPUT; // MacFlag:Input is 0 
    }
    
    // Increment MacCount before operation. Invalidate nonce if MacCount has reached max
    if (param->nonce->value[12] == 255) {
        param->nonce->valid = false;
    }
    param->nonce->value[12]++;
    
    // Do the cryptographic calculation
    // Check if additional auth-only field is used
    aes132crypt_param.alen_bytes = (param->mode & 0xE0)? 30: 14;
    
    // The length is passed in "Param2" for encryption commands, and 16 for KeyLoad
    if ((param->opcode == AES132_OPCODE_ENC_WRITE) || (param->opcode == AES132_OPCODE_DECRYPT)) {
        aes132crypt_param.plen_bytes = param->param2 & 0xFF;
    } else if (param->opcode == AES132_OPCODE_KEY_LOAD) {
        aes132crypt_param.plen_bytes = 16;
    } else {
        aes132crypt_param.plen_bytes = 0;
    }
    
    // Fill in the rest of parameters
    aes132crypt_param.nonce = param->nonce->value;
    aes132crypt_param.associated_data = associated_data;    
    aes132crypt_param.key = param->key;
    aes132crypt_param.payload = param->in_data; 
    aes132crypt_param.auth_value = param->out_mac;
    aes132crypt_param.ciphertext = param->out_data;
    
    // Execute...
    ret_code = aes132crypt_generate_encrypt(&aes132crypt_param);
    if (ret_code) {
        return ret_code;
    }
    
    return AES132_FUNCTION_RETCODE_SUCCESS;
}

/** \brief This function checks authentication/integrity MAC and decrypts data from ATAES132 commands.
 * 
 * Operations supported by this function:
 * - Check OutMac from Auth, Outbound-only (Mode[1:0] = 10b)
 * - Check OutMac from Auth, Mutual (Mode[1:0] = 11b)
 * - Check OutMac from Counter read (Mode[1:0] = 11b)
 * - Decrypt OutData and check OutMac from EncRead
 * - Decrypt OutData and check OutMac from Encrypt
 * - Decrypt OutData and check OutMac from KeyCreate
 * - Decrypt OutData and check OutMac from KeyExport (Mode[0] = 0b)
 * 
 * This function does _not_ support:
 * - EncRead command configuration memory signature MAC (Appendix I.17)
 * - EncRead command key memory signature MAC (Appendix I.18) 
 *
 * All unused pointer in parameter struct can take NULL value.
 * The use of second authentication blocks (usage_counter, serial_num, small_zone) is determined by mode bit 7-5.
 * count_value is used only for Counter command, and not used for other commands.
 * out_mac is never used for this function.
 * 
 * User must provide enough space for output parameters (out_data).
 * 
 * The "nonce" struct has to be passed with valid member set to true, otherwise the function returns with error.
 * The "nonce" struct is checked and updated by this function:
 * - If MacCount has reached maximum value (255), valid flag is cleared
 * - MacCount (nonce[12]) is incremented
 * - If MAC compare fails, valid flag is cleared, and MacCount is set to zero
 * 
 * The function returns with bad parameter error if:
 * - Passed with opcode/mode combination that is not supported
 * - At least one of used pointer parameters has NULL value
 *
 * \param [in,out] param Pointer to structure for input/output parameters. Refer to aes132h_in_out.
 * \return status of the operation. 
 */
uint8_t aes132h_mac_check_decrypt(struct aes132h_in_out *param)
{
    uint8_t ret_code;
    struct aes132crypt_in_out aes132crypt_param;    
    uint8_t associated_data[30];
    
    // Check parameters
    if( !param->key ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt: No Key provided");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }
    else if( !param->nonce ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt: No nonce provided");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }
    else if( !param->in_mac ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt: No in_mac provided");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }
    else if( (param->mode & 0x20) && !param->usage_counter ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt:  "
                "usage_counter specified but not provided");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }
    else if( (param->mode & 0x40) && !param->serial_num ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt: "
                "serial_num specified but not provided");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }
    else if( (param->mode & 0x80) && !param->small_zone ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt: "
                "small_zone specified but not provided");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }
    else if( (AES132_OPCODE_COUNTER == param->opcode) && !param->count_value ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt: "
                "Counter op_code specified but not provided"); // todo: CHECK THIS
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }
    else if( ((AES132_OPCODE_ENC_READ == param->opcode)
            || (AES132_OPCODE_ENCRYPT == param->opcode)
            || (AES132_OPCODE_KEY_CREATE == param->opcode)) && (!param->in_data || !param->out_data) ) {
        ESP_LOGE(TAG, "aes132h_mac_check_decrypt: "
                "ENC_READ, ENCRYPT, or KEY_CREATE opcode specified. "
                "in_data and/or out_data not provided.");
        return AES132_FUNCTION_RETCODE_BAD_PARAM;
    }

    // Check nonce validity
    if (param->nonce->valid == false) {
        return AES132_DEVICE_RETCODE_NONCE_ERROR;
    }
    
    // Arrange associated_data (see ATAES132 datasheet appendix I)
    associated_data[0] = AES132_MANUFACTURING_ID >> 8;
    associated_data[1] = AES132_MANUFACTURING_ID & 0xFF;
    associated_data[2] = param->opcode;
    associated_data[3] = param->mode;
    associated_data[4] = param->param1 >> 8;
    associated_data[5] = param->param1 & 0xFF;
    associated_data[6] = param->param2 >> 8;
    associated_data[7] = param->param2 & 0xFF;
    associated_data[8] = (param->nonce->random)? AES132_MAC_FLAG_RANDOM: 0x00;   // Input bit (bit 1) always 0
    if (param->opcode == AES132_OPCODE_COUNTER) {
        memcpy(&associated_data[9], param->count_value, 4);
    } else {
        memset(&associated_data[9], 0x00, 4);
    }
    associated_data[13] = 0x00;
    
    // Append additional associated_data if specified by mode parameter
    if (param->mode & 0x20) {
        ESP_LOGI(TAG, "Including Counter in nonce computation.");
        memcpy(&associated_data[14], param->usage_counter, 4);
    } else {
        memset(&associated_data[14], 0x00, 4);
    }
    if (param->mode & 0x40) {
        ESP_LOGI(TAG, "Including SerialNum in nonce computation.");
        memcpy(&associated_data[18], param->serial_num, 8);
    } else {
        memset(&associated_data[18], 0x00, 8);
    }
    if (param->mode & 0x80) {
        ESP_LOGI(TAG, "Including SmallZone in nonce computation.");
        memcpy(&associated_data[26], param->small_zone, 4);
    } else {
        memset(&associated_data[26], 0x00, 4);
    }
    
    // Increment MacCount before operation. Invalidate nonce if MacCount has reached max
    if (param->nonce->value[12] == 255) {
        param->nonce->valid = false;
    }
    param->nonce->value[12]++;
    ESP_LOGI(TAG, "Incrementing mac_count to %d", param->nonce->value[12]);
    
    // Do the cryptographic calculation
    // Check if additional auth-only field is used
    aes132crypt_param.alen_bytes = (param->mode & 0xE0)? 30: 14;

    // The length is passed in "Param2" for encryption commands, and 16 for KeyCreate/KeyExport
    if (       (param->opcode == AES132_OPCODE_ENC_READ) 
            || (param->opcode == AES132_OPCODE_ENCRYPT) ) {
        aes132crypt_param.plen_bytes = param->param2 & 0xFF;
    } else if ( param->opcode == AES132_OPCODE_KEY_CREATE ) {
        aes132crypt_param.plen_bytes = 16;
    } else {
        aes132crypt_param.plen_bytes = 0;
    }
    
    // Fill in the rest of parameters
    aes132crypt_param.nonce           = param->nonce->value;
    aes132crypt_param.associated_data = associated_data;
    aes132crypt_param.key             = param->key;
    aes132crypt_param.payload         = param->out_data;
    aes132crypt_param.auth_value      = param->in_mac;
    aes132crypt_param.ciphertext      = param->in_data;
    
    // Execute...
    ret_code = aes132crypt_decrypt_verify(&aes132crypt_param);
    if (ret_code == AES132_DEVICE_RETCODE_MAC_ERROR) {
        // Invalidate nonce on MAC mismatch
        param->nonce->valid = false;
        param->nonce->value[12] = 0;
    }
    
    return ret_code;
}


