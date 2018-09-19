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
 *  \brief  Declarations and Prototypes for AES132 Helper Functions
 *  \author Tuwuh Sarwoprasojo, Atmel Bali Team
 *  \date   June 12, 2013
 */

/** \mainpage AES132 Helper Functions
 * The AES132 Helper Functions provides implementation of host-side functions for AES132.
 * This includes aes132crypt_* functions which perform AES-CCM and nonce calculation.
 * 
 * The AES132 nonce is implemented as a structure aes132h_nonce_s.
 * The structure member "value" contains concatenation of 12-byte "Nonce" and 1-byte "MacCount". MacCount can be accessed in value[12].
 * The value of MacCount is reset to 0 by nonce calculation function upon success. MacCount is incremented prior to any computation performed.
 * Nonce validity is stored in "valid" structure member. A TRUE indicates that the nonce is valid, a FALSE indicates that it is invalid.
 * "valid" is set to TRUE by nonce calculation function upon success.
 * After 255 calculations (MacCount has reached maximum value), "valid" is set to FALSE by any function using it.
 * 
 * ManufacturingID is defined as a AES132_MANUFACTURING_ID macro in the header file.
 * User must set this macro definition according to the content of ManufacturingID register in the particular ATAES132 configuration memory (address 0xF02B-0xF02C, MSByte is in 0xF02B).
 * 
 * There are 6 public functions:
 * -# Nonce generation, aes132h_nonce()
 * -# CountValue conversion (decode 4-byte CountValue to integer counts), aes132h_decode_count_value()
 * -# Counter personalization (encode integer counts to 8-byte Counter field), aes132h_encode_counter_field()
 * -# FirstBlock and SecondBlock builder, aes132h_build_auth_block()
 * -# MAC computation and data encryption, aes132h_mac_compute_encrypt()
 * -# MAC checking and data decryption, aes132h_mac_check_decrypt()
 * 
 * Please refer to each function documentation for details.
 */
 
#ifndef AES132_HELPER_H_
#define AES132_HELPER_H_

#include <stdint.h>
#include <stdbool.h>

//-------------------
// Macro definitions
//-------------------

#ifndef AES132_FUNCTION_RETCODE_BAD_PARAM
	#define AES132_FUNCTION_RETCODE_BAD_PARAM      ((uint8_t) (0xE2))
#endif

#ifndef AES132_MANUFACTURING_ID
	#define AES132_MANUFACTURING_ID                (0x00EE)
#endif

#ifndef AES132_COUNTER_MAX
	#define AES132_COUNTER_MAX                     (2097151)
#endif

//! Additional macro definitions for use with aes132 library
// --- Address ---
// User memory
#define AES132_USER_ZONE_ADDR(n)      ((uint16_t) (0x0000 + ((n) << 8)))

// Command memory
//   has been defined in aes132.h

// Configuration memory
#define AES132_SERIAL_NUM_ADDR        ((uint16_t) 0xF000)
#define AES132_MANUFACTURING_ID_ADDR  ((uint16_t) 0xF02B)
#define AES132_LOCK_KEYS_ADDR         ((uint16_t) 0xF020)
#define AES132_LOCK_SMALL_ADDR        ((uint16_t) 0xF021)
#define AES132_LOCK_CONFIG_ADDR       ((uint16_t) 0xF022)

#define AES132_KEY_CONFIG_ADDR(n)     ((uint16_t) (0xF080 + ((n) << 2)))
#define AES132_ZONE_CONFIG_ADDR(n)    ((uint16_t) (0xF0C0 + ((n) << 2)))
#define AES132_COUNTER_CONFIG_ADDR(n) ((uint16_t) (0xF060 + ((n) << 1)))
#define AES132_COUNTER_ADDR(n)        ((uint16_t) (0xF100 + ((n) << 3)))

#define AES132_SMALL_ZONE_ADDR        ((uint16_t) (0xF1E0))

// Key memory
#define AES132_KEY_ADDR(n)            ((uint16_t) (0xF200 + ((n) << 4)))

// --- Length ---
#define AES132_USER_ZONE_LENGTH          (256)
#define AES132_SERIAL_NUM_LENGTH         (8)
#define AES132_MANUFACTURING_ID_LENGTH   (2)
#define AES132_KEY_CONFIG_LENGTH         (4)
#define AES132_ZONE_CONFIG_LENGTH        (4)
#define AES132_COUNTER_CONFIG_LENGTH     (2)
#define AES132_COUNTER_LENGTH            (8)
#define AES132_KEY_LENGTH                (16)

#define AES132_MAC_LENGTH                (16)


// --- Bit positions ---
// ChipConfig Byte #0
#define AES132_CHIP_CONFIG_LEGACY_EN          (1 << 0)
#define AES132_CHIP_CONFIG_ENC_DEC_EN         (1 << 1)
#define AES132_CHIP_CONFIG_DEC_READ_EN        (1 << 2)
#define AES132_CHIP_CONFIG_AUTH_COMPUTE_EN    (1 << 3)
// Warning; only select one of the following power configurations
#define AES132_CHIP_CONFIG_POWER_ACTIVE       ((1 << 6) | (1 << 7))
#define AES132_CHIP_CONFIG_POWER_STANDBY      (1 << 6)
#define AES132_CHIP_CONFIG_POWER_SLEEP        0

// KeyConfig Byte #0
#define AES132_KEY_CONFIG_EXTERNAL_CRYPTO     (1 << 0)
#define AES132_KEY_CONFIG_INBOUND_AUTH        (1 << 1)
#define AES132_KEY_CONFIG_RANDOM_NONCE        (1 << 2)
#define AES132_KEY_CONFIG_LEGACY_OK           (1 << 3)
#define AES132_KEY_CONFIG_AUTH_KEY            (1 << 4)
#define AES132_KEY_CONFIG_CHILD               (1 << 5)
#define AES132_KEY_CONFIG_PARENT              (1 << 6)
#define AES132_KEY_CONFIG_CHANGE_KEYS         (1 << 7)

// KeyConfig Byte #1
#define AES132_KEY_CONFIG_COUNTER_LIMIT       (1 << 0)
#define AES132_KEY_CONFIG_CHILD_MAC           (1 << 1)
#define AES132_KEY_CONFIG_AUTH_OUT            (1 << 2)
#define AES132_KEY_CONFIG_AUTH_OUT_HOLD       (1 << 3)
#define AES132_KEY_CONFIG_IMPORT_OK           (1 << 4)
#define AES132_KEY_CONFIG_EXPORT_AUTH         (1 << 5)
#define AES132_KEY_CONFIG_TRANSFER_OK         (1 << 6)
#define AES132_KEY_CONFIG_AUTH_COMPUTE        (1 << 7)

// KeyConfig Byte #2
#define AES132_KEY_CONFIG_COUNTER_NUM(x)      (x << 4)
#define AES132_KEY_CONFIG_LINK_POINTER(x)     (x)

// KeyConfig Byte #3
#define AES132_KEY_CONFIG_DEC_READ            (1 << 0)
// CounterConfig Byte #0
#define AES132_COUNTER_CONFIG_INCREMENT_OK    (1 << 0)
#define AES132_COUNTER_CONFIG_REQUIRE_MAC     (1 << 1)

// ZoneConfig Byte #0
#define AES132_ZONE_CONFIG_AUTH_READ          (1 << 0)
#define AES132_ZONE_CONFIG_AUTH_WRITE         (1 << 1)
#define AES132_ZONE_CONFIG_ENC_READ           (1 << 2)
#define AES132_ZONE_CONFIG_ENC_WRITE          (1 << 3)
#define AES132_ZONE_CONFIG_WRITE_MODE_4       (1 << 4)
#define AES132_ZONE_CONFIG_WRITE_MODE_5       (1 << 5)
#define AES132_ZONE_CONFIG_USE_SERIAL         (1 << 6)
#define AES132_ZONE_CONFIG_USE_SMALL          (1 << 7)

// ZoneConfig Byte #1
#define AES132_ZONE_CONFIG_AUTH_ID(x)         (x << 4)
#define AES132_ZONE_CONFIG_READ_ID(x)         (x)

// ZoneConfig Byte #2
#define AES132_ZONE_CONFIG_WRITE_ID(x)        (x << 4)
#define AES132_ZONE_CONFIG_VOLATILE_TRANSFER_OK (1 << 0)

// Zone Config Byte #3
#define AES132_ZONE_CONFIG_READ_ONLY_R        0
#define AES132_ZONE_CONFIG_READ_ONLY_RW       ((uint8_t) 0x55)

// VolUsage Byte #0
#define AES132_VOL_USAGE_AUTH_OK              (1 << 0)
#define AES132_VOL_USAGE_ENCRYPT_OK_1         (1 << 1)
#define AES132_VOL_USAGE_ENCRYPT_OK_2         (1 << 2)
#define AES132_VOL_USAGE_DECRYPT_OK           (1 << 3)
#define AES132_VOL_USAGE_RANDOM_NONCE         (1 << 4)
#define AES132_VOL_USAGE_AUTH_COMPUTE         (1 << 5)
#define AES132_VOL_USAGE_LEGACY_OK            (1 << 6)
#define AES132_VOL_USAGE_EXPORT_OK            (1 << 7)

// VolUsage Byte #1
#define AES132_VOL_USAGE_WRITE_COMPUTE        (1 << 0)
#define AES132_VOL_USAGE_DEC_READ             (1 << 1)

// AuthUsage
#define AES132_AUTH_USAGE_READ_OK             (1 << 0)
#define AES132_AUTH_USAGE_WRITE_OK            (1 << 1)
#define AES132_AUTH_USAGE_KEY_USE             (1 << 2)

// MacFlag
#define AES132_MAC_FLAG_RANDOM                (1 << 0)
#define AES132_MAC_FLAG_INPUT                 (1 << 1)

// Jolt Key ID Usage
#define AES132_KEY_ID_MASTER   ((uint8_t) 0)
#define AES132_KEY_ID_STRETCH  ((uint8_t) 1)
#define AES132_KEY_ID_PIN(x)   ((uint8_t) (2+x))


//----------------------
// Structures for nonce
//----------------------
/** \struct aes132h_nonce_s
 *  \brief Implementation of AES132 nonce.
 *  \var aes132h_nonce_s::value
 *       \brief 13-byte array, concatenation of 12-byte "Nonce" and 1-byte "MacCount".
 *              MacCount can be accessed in value[12].
 *  \var aes132h_nonce_s::valid
 *       \brief 1-byte flag, indicating nonce validity. Possible value: TRUE or FALSE.
 *  \var aes132h_nonce_s::random
 *       \brief 1-byte flag, indicating whether the nonce is generated from random number.
 *              Possible value: TRUE or FALSE.
 */
struct aes132h_nonce_s {
	uint8_t value[13];
	uint8_t valid;
	uint8_t random;
};

//----------------------------------------
// Structures for input/output parameters
//----------------------------------------
/** \struct aes132crypt_in_out
 *  \brief Input/output parameters for function aes132crypt_generate_encrypt() and aes132crypt_decrypt_verify().
 *  \var aes132crypt_in_out::nonce
 *       \brief Pointer to 13-byte nonce for AES-CCM. For AES132, this is 12-byte Nonce concatenated with 1-byte MacCount.
 *  \var aes132crypt_in_out::associated_data
 *       \brief Pointer to associated data for AES-CCM calculation. For AES132, this is used in all commands. In the AES132 datasheet this is described as "authenticate-only data".
 *  \var aes132crypt_in_out::alen_bytes
 *       \brief Length of associated data, in bytes. For AES132, depending on mode used in the command, this can be 16 or 32.
 *  \var aes132crypt_in_out::key
 *       \brief Pointer to 16-byte key used in AES cipher.
 *  \var aes132crypt_in_out::payload
 *       \brief Pointer to payload for AES-CCM calculation. For AES132, this is used in commands requiring encryption. In the AES132 datasheet this is described as "cleartext data". It can be set to NULL if not used (plen_bytes = 0).
 *              This is input for aes132crypt_generate_encrypt(), and output for aes132crypt_decrypt_verify()
 *  \var aes132crypt_in_out::plen_bytes
 *       \brief Length of payload, in bytes. For AES132, depending on the command, this can be between 0 to 32.
 *  \var aes132crypt_in_out::auth_value
 *       \brief Pointer to 16-byte authentication value. In AES132 datasheet this is described as "input integrity MAC".
 *              This is output for aes132crypt_generate_encrypt(), and input for aes132crypt_decrypt_verify() 
 *  \var aes132crypt_in_out::ciphertext
 *       \brief Pointer to ciphertext output. For AES132, this can be 16 or 32 bytes. Caller must provide enough space.
 *              This is output for aes132crypt_generate_encrypt(), and input for aes132crypt_decrypt_verify() 
 */
struct aes132crypt_in_out {
	uint8_t *nonce;
	uint8_t *associated_data;
	uint8_t alen_bytes;
	uint8_t *key;
	uint8_t *payload;
	uint8_t plen_bytes;
	uint8_t *auth_value;
	uint8_t *ciphertext;
};

/** \struct aes132crypt_ctr_block_in_out
 *  \brief Input/output parameters for function aes132crypt_generate_encrypt().
 *  \var aes132crypt_ctr_block_in_out::auth_value_in
 *       \brief Pointer to 16-byte input authentication value (MAC).
 *  \var aes132crypt_ctr_block_in_out::data_in
 *       \brief Pointer to input data to be encrypted/decrypted.
 *  \var aes132crypt_ctr_block_in_out::plen_bytes
 *       \brief Length of the data (payload), in bytes.
 *  \var aes132crypt_ctr_block_in_out::nonce
 *       \brief Pointer to 13-byte nonce (called N in NIST Special Publication 800-38C).
 *  \var aes132crypt_ctr_block_in_out::key
 *       \brief Pointer to 16-byte key (called K in NIST Special Publication 800-38C).
 *  \var aes132crypt_ctr_block_in_out::auth_value_out
 *       \brief Pointer to 16-byte output authentication value (MAC).
 *  \var aes132crypt_ctr_block_in_out::data_out
 *       \brief Pointer to encrypted/decrypted data.
 */
struct aes132crypt_ctr_block_in_out {
	uint8_t *auth_value_in;
	uint8_t *data_in;
	uint8_t plen_bytes;
	uint8_t *nonce;
	uint8_t *key;
	uint8_t *auth_value_out;
	uint8_t *data_out;
};

/** \struct aes132crypt_nonce_in_out
 *  \brief Input/output parameters for function aes132crypt_nonce().
 *  \var aes132crypt_nonce_in_out::data_in
 *       \brief Pointer to 16-byte input message to cipher, in datasheet stated as block A.
 *  \var aes132crypt_nonce_in_out::key"
 *       \brief Pointer to 16-byte key, in datasheet stated as block B.
 *  \var aes132crypt_nonce_in_out::data_out"
 *       \brief Pointer to 16-byte output.
 */
struct aes132crypt_nonce_in_out {
	uint8_t *data_in;
	uint8_t *key;
	uint8_t *data_out;
};

/** \struct aes132h_nonce_in_out
 *  \brief Input/output parameters for function aes132h_nonce().
 *  \var aes132h_nonce_in_out::mode
 *       \brief [in]  8-bit integer, Mode argument used in ATAES132 Nonce command.
 *                    If Bit 0 = 1b, generates a random nonce based on in_seed and random,
 *                    if bit 0 = 0b, stores in_seed as the nonce.
 *  \var aes132h_nonce_in_out::in_seed
 *       \brief [in]  Pointer to 12-byte input seed chosen by application.
 *  \var aes132h_nonce_in_out::random
 *       \brief [in]  Pointer to 16-byte random number generated from the device (if used).
 *  \var aes132h_nonce_in_out::nonce
 *       \brief [out] Pointer to aes132h_nonce_s struct, as output for the resulting nonce.
 */
struct aes132h_nonce_in_out {
	uint8_t mode;
	uint8_t *in_seed;
	uint8_t *random;
	struct aes132h_nonce_s *nonce;
};


/** \struct aes132h_build_auth_block_in_out
 *  \brief Input/output parameters for function aes132h_build_auth_block().
 *  \var aes132h_build_auth_block_in_out::host_opcode
 *       \brief [in]  8-bit integer host Opcode to send the FirstBlock/SecondBlock to (e.g. AuthCompute, KeyExport).
 *  \var aes132h_build_auth_block_in_out::opcode
 *       \brief [in]  8-bit integer Opcode argument to be assembled into FirstBlock (e.g. Auth, KeyLoad).
 *  \var aes132h_build_auth_block_in_out::mode
 *       \brief [in]  8-bit integer Mode argument to be assembled into FirstBlock.
 *  \var aes132h_build_auth_block_in_out::param1
 *       \brief [in]  16-bit integer Param1 argument to be assembled into FirstBlock.
 *  \var aes132h_build_auth_block_in_out::param2
 *       \brief [in]  16-bit integer Param2 argument to be assembled into FirstBlock.
 *  \var aes132h_build_auth_block_in_out::count_value
 *       \brief [in]  Pointer to 4-byte CountValue to be assembled into FirstBlock. Only used for Counter opcode.
 *  \var aes132h_build_auth_block_in_out::usage_counter
 *       \brief [in]  Pointer to usage counter value to be assembled into SecondBlock, if specified by mode parameter.
 *  \var aes132h_build_auth_block_in_out::serial_num
 *       \brief [in]  Pointer to SerialNum[0:7], to be assembled into SecondBlock, if specified by mode parameter.
 *  \var aes132h_build_auth_block_in_out::small_zone
 *       \brief [in]  Pointer to SmallZone[0:3], to be assembled into SecondBlock, if specified by mode parameter.
 *  \var aes132h_build_auth_block_in_out::random
 *       \brief [in]  8-bit integer for Random flag. 0x01 if using random nonce, 0x00 if using inbound nonce.
 *  \var aes132h_build_auth_block_in_out::first_block
 *       \brief [out] Pointer to 6- or 11-byte FirstBlock output.
 *  \var aes132h_build_auth_block_in_out::second_block
 *       \brief [out] Pointer to 16-byte SecondBlock output, if used.
 */
struct aes132h_build_auth_block_in_out {
	uint8_t host_opcode;
	uint8_t opcode;
	uint8_t mode;
	uint16_t param1;
	uint16_t param2;
	uint8_t *count_value;
	uint8_t *usage_counter;
	uint8_t *serial_num;
	uint8_t *small_zone;
	uint8_t random;
	uint8_t *first_block;
	uint8_t *second_block;
};


/** \struct aes132h_in_out
 *  \brief Input/output parameters for function aes132h_mac_compute_encrypt() and aes132h_mac_check_decrypt().
 *  \var aes132h_in_out::opcode        
 *       \brief [in]     8-bit integer Opcode argument used in ATAES132 command.
 *  \var aes132h_in_out::mode          
 *       \brief [in]     8-bit integer Mode argument used in ATAES132 command.
 *  \var aes132h_in_out::param1        
 *       \brief [in]     16-bit integer Param1 argument used in ATAES132 command.
 *  \var aes132h_in_out::param2        
 *       \brief [in]     16-bit integer Param2 argument used in ATAES132 command.
 *                       For EncWrite and Decrypt, this is the Count field, representing number of bytes to encrypt.
 *  \var aes132h_in_out::count_value   
 *       \brief [in]     Pointer to 4-byte CountValue. Only used for Counter opcode.
 *  \var aes132h_in_out::usage_counter 
 *       \brief [in]     Pointer to usage counter value to be used in second authenticate-only block.
 *  \var aes132h_in_out::serial_num    
 *       \brief [in]     Pointer to SerialNum[0:7], to be used in second authenticate-only block.
 *  \var aes132h_in_out::small_zone    
 *       \brief [in]     Pointer to SmallZone[0:3], to be used in second authenticate-only block.
 *  \var aes132h_in_out::key           
 *       \brief [in]     Pointer to 16-byte key used to calculate the MAC.
 *  \var aes132h_in_out::nonce         
 *       \brief [in,out] Pointer to aes132h_nonce_s structure.
 *  \var aes132h_in_out::in_mac        
 *       \brief [in]     Pointer to 16-byte input MAC to be checked by the function.
 *  \var aes132h_in_out::out_mac       
 *       \brief [out]    Pointer to 16-byte output MAC generated by the function.
 *  \var aes132h_in_out::in_data       
 *       \brief [in]     Pointer to input data to be encrypted, or decrypted.
 *                       Not used for Auth and Counter command.
 *  \var aes132h_in_out::out_data      
 *       \brief [out]    Pointer to 16- or 32-byte encrypted output data; or pointer to decrypted output data.
 *                       Not used for Auth and Counter command.
 */
struct aes132h_in_out {
	uint8_t opcode;
	uint8_t mode;
	uint16_t param1;
	uint16_t param2;
	uint8_t *count_value;
	uint8_t *usage_counter;
	uint8_t *serial_num;
	uint8_t *small_zone;
	uint8_t *key;
	struct aes132h_nonce_s *nonce;
	uint8_t *in_mac;
	uint8_t *out_mac;
	uint8_t *in_data;
	uint8_t *out_data;
};

//---------------------
// Function prototypes
//---------------------
uint8_t aes132h_nonce(struct aes132h_nonce_in_out *param);
uint8_t aes132h_decode_count_value(uint8_t *count_value, uint32_t *count_integer);
uint8_t aes132h_encode_counter_field(uint32_t count_integer, uint8_t *counter_field);
uint8_t aes132h_build_auth_block(struct aes132h_build_auth_block_in_out *param);
uint8_t aes132h_mac_compute_encrypt(struct aes132h_in_out *param);
uint8_t aes132h_mac_check_decrypt(struct aes132h_in_out *param);

#endif //AES132_HELPER_H_
