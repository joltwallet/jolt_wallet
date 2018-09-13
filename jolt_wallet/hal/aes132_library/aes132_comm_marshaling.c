// ----------------------------------------------------------------------------
//         ATMEL Crypto-Devices Software Support  -  Colorado Springs, CO -
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
 *  \brief Command Marshaling Layer of ATAES132 Library
 *  \author Atmel Crypto Products
 *  \date   June 08, 2015
 */


#include <string.h>                    // needed for memcpy()
#include "aes132_comm_marshaling.h"    // definitions and declarations for the Command Marshaling module
#include "aes132_i2c.h" // For ReturnCode macros
#include "i2c_phys.h" // For res macros

#include "esp_system.h"
#include "esp_log.h"
#include "sodium.h"
#include "hal/storage.h"

static const char TAG[] = "aes132m";
static uint8_t *master_key = NULL;

/* Used to mirror the internal MacCount of the ataes132a device.
 * The ataes132a MacCount gets zero'd when:
 *     * Nonce command is executed
 *     * A MAC compare operation fails.
 *     * MacCount reaches the maximum count.
 *     * A Reset event occurs: 
 *          * Power-Up (see Appendix L ChipState = Power-Up)
 *          * Wake-Up from Sleep (see Appendix L. ChipState = Wake-Up from Sleep)
 *          * Reset command (see Section 7.22 Reset Command)
 *          * Security Tamper is activated, causing the hardware to reset
 * The ataes132a MacCount gets incremented when:
 *     * prior to any MAC computation being performed
 * E.g. the value that will be used for calculating the first MAC of the first 
 * command after the Nonce command is MAC=1.
 * Can be read via the INFO command.
 * For more information, see Page 107
 */
static uint8_t mac_count = 0;
static uint8_t nonce[12] = { 0 };

#define MAC_COUNT_LOCKSTEP_CHECK false
static uint8_t mac_incr() {
    /* Increments the local maccount, sends a nonce command if it needs to
     * be reset */

#if MAC_COUNT_LOCKSTEP_CHECK
    {
        // Read the aes132m mac_count to make sure we are in lock-step
        uint8_t device_count;
        aes132m_mac_count(&device_count);
        if( device_count != mac_count ) {
            ESP_LOGE(TAG, "mac_count desyncronized; Device=%d; Local=%d",
                    device_count, mac_count);
        }
    }
#endif
    if( UINT8_MAX == mac_count ) {
        // todo: error handling, nonce sync?
        ESP_LOGD(TAG, "local mac_count maxed out, issuing rand Nonce");
        uint8_t res = aes132m_nonce(NULL, NULL);
        mac_count = 0;
    }
    mac_count++; // Since The Encrypt command returns a MAC, increase mac_count
    return mac_count;
}
/* See Page 110
 * There are two passes through the AES crypto engine in CBC mode to create 
 * the cleartext MAC. The inputs to the crypto engine for those blocks are 
 * labeled B0 and B1, and the outputs are B’0 and B’1, respectively.
 *
 * B0 is composed of the following 128 bits: 
 *     – 1 byte flag, a fixed value of b0111 1001. 
 *     – 12 byte Nonce, as generated by the Nonce command. 
 *     – 1 byte MacCount, one for first MAC generation. 
 *     – 2 byte length field, always 0x00 00 for authentication only.
 * B1 is the XOR of B’0 with the following 128 bits:
 *     – 2 byte length field, size of authenticate-only data.
 *     – 14 byte data to be authenticated only.
 * B’1 is the cleartext MAC, which must be encrypted before being sent to the system
 *
 */
static void create_b0(uint8_t *b0, uint16_t data_len){
    /* Crafts bits into 16-byte output buffer b0 
     * Uses the global nonce and mac_count*/
    b0[0] = 0x79; // constant flag
    memcpy(&b0[1], nonce, 12); // nonce data
    b0[13] = mac_count; // make sure to increment mac_count before performing all these operations
    memcpy(&b0[14], &data_len, 2); // 2-byte length field; check back on this
}

static void create_b1(const uint8_t *bp0, uint8_t *b1, uint16_t data_len, uint8_t *data) {
    /* Crafts bit into 16-byte output buffer b1 */
    memcpy(&b1[0], &data_len, 2);
    if( data_len > 14 ) {
        memcpy(&b1[2], data, 14);
    }
    else {
        memcpy(&b1[2], data, data_len); // Check on this, do we zero pad the data?
    }
    // Perform the XOR operation on the arrays
    for(uint8_t i=0; i<16; i++){
        b1[i] ^= bp0[i];
    }
}

static void create_a0(uint8_t *a0) {
    /* Crafts bits into 16-byte output buffer a0*/
    a0[0] = 0x01; // constant flag
    memcpy(&a0[1], nonce, 12); // nonce data
    a0[13] = mac_count; // make sure to increment mac_count before performing all these operations
    a0[14] = 0x00; // constant
    a0[15] = 0x00; // constant
}

static bool check_configlock() {
    /* Returns true if device configuratoin is locked */
    uint8_t data;
    uint8_t res;
    res =  aes132m_blockread(&data, AES132_LOCKCONFIG_ADDR, sizeof(data));
    if( 0x55 == data ) {
        // Device configuration is unlocked
        return false;
    }
    else {
        return true;
    }
}


#ifdef UNIT_TESTING
void aes132m_debug_set_local_mac_count( uint8_t count ) {
    mac_count = count;
}

void aes132m_debug_set_local_nonce( uint8_t *val ) {
    memcpy(nonce, val, sizeof(nonce));
}

void aes132m_debug_reset_mac_count() {
    aes132m_debug_set_local_mac_count(0);
    // todo
}

void aes132m_debug_set_nonce( uint8_t *val ) {
    // todo
}

uint8_t aes132m_debug_clear_master_key() {
    // deallocate master_key space
    if( NULL != master_key ) {
        sodium_free(master_key);
        master_key = NULL;
    }
    // todo: delete the master_key in esp32 storage

    // Clear backed up masterkey
    if( check_configlock() ) {
        // Device configuration is locked, cannot clear backup
        return 1;
    }
    else {
        // Device configuration is unlocked, can clear backup
        aes132_reset_master_zoneconfig();
        uint8_t zeros[16] = { 0 };
        uint8_t res = aes132m_write_memory(sizeof(zeros),
                AES132_USERZONE(0), zeros);
    }
    return 0;
}
#endif
#if 0
uint8_t aes132m_debug_auth_compute(uint8_t *out_mac, const uint8_t key_id,
        const uint8_t *b0, const uint8_t *b1) {
    /* Computes and returns the 16-byte out_mac */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t b0[16] = { 0 };
    uint8_t b1[16] = { 0 };

    create_b0();
    create_b1();

    cmd = AES132_AUTH_COMPUTE;
    mode = 0x00; // Reserved; Always Zero
    param1 = (uint8_t) key_id;
    param2 = 0x0000; // Reserved; Always Zero
    res = aes132m_execute(cmd, mode, param1, param2,
            16, b0, 16, b1, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    memcpy(out_mac, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
    return 0; // success
}
#endif

static void lock_device() {
    /* Locks smallzone, config memory, key memory, and makes the master
     * UserZone read-only 
     *
     * Warning; this function has permament physical impacts.
     * */
    // Configuration Memory MUST be locked before Key Memory
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};

    const uint8_t cmd = AES132_LOCK;
    // Require Validate memory checksum in Param2
    const uint8_t mode_common = 0x04 | AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER;

    /* Lock SmallZone */
    {
        ESP_LOGI(TAG, "Locking SmallZone");
        const uint8_t mode = mode_common | AES132_LOCK_SMALLZONE;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2;
        uint8_t res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    }
    /* Make Master UserZone Read-Only */
    {
        ESP_LOGI(TAG, "Locking UserZone 0 (master) read-only");
        const uint8_t mode = mode_common | AES132_LOCK_ZONECONFIG_RO;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2;

        uint8_t data[16];
        uint8_t res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

    }
    /* Lock Configuration Memory */
    {
        ESP_LOGI(TAG, "Locking Configuration Memory");
        uint8_t mode = mode_common | AES132_LOCK_CONFIG;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2;
        uint8_t res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    }
    /* Lock Key Memory */
    {
        ESP_LOGI(TAG, "Locking Key Memory");
        const uint8_t mode = mode_common | AES132_LOCK_KEY;
        uint16_t param1 = 0;
        // todo: compute checksum into param2
        uint16_t param2;
        uint8_t res = aes132m_execute(cmd, mode, param1, param2,
                0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    }
}

uint8_t aes132m_load_master_key() {
    /* Allocates space for the master key on the heap 
     * If device is unlocked:
     *     * Generate Master Key from ESP32 Entropy
     *     * Configure Device
     *     * Encrypt and Backup Master Key to MasterUserZone
     *     * Write key to slot 0
     *     * Lock Device
     * If device is locked:
     *     * Load and decrypt Master key from MasterUserZone
     */
    uint8_t res;
    if( NULL==master_key ) {
        master_key = sodium_malloc(16);
    }
    if( NULL==master_key ){
        ESP_LOGE(TAG, "Unable to allocate space for the ATAES132A Master Key");
        esp_restart();
    }
    sodium_mprotect_readwrite(master_key);
    sodium_memzero(master_key, 16);

    /* Check if the device is locked, if not locked generate a new master key */ 
    ESP_LOGI(TAG, "Checking if device is locked");
    if( check_configlock() ) {
        // Device configuration is locked
        /* Attempt to load key from encrypted spi flash */
        /* Loads the key from storage */
        // todo
        /* If not found in storage, check the Master UserZone */
        {
            uint8_t rx[16];
            res = aes132m_read_memory(sizeof(rx), AES132_USERZONE(0),
                    rx);
            ESP_LOGI(TAG, "Read memory result: %d", res);
            ESP_LOGI(TAG, "Confidential; "
                    "ATAES132A Master Key Backup Response: 0x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                    rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7],
                    rx[8], rx[9], rx[10], rx[11], rx[12], rx[13], rx[14],
                    rx[15]);
            //todo decrypt rx; here just identity
            memcpy(master_key, rx, 16);
        }
    }
    else {
        // Device configuration is unlocked
        /* Generate new master key */
        for( uint8_t i=0; i<4; i++ ) {
            uint32_t entropy = randombytes_random();
            memcpy(&((uint32_t*)master_key)[i], &entropy, sizeof(uint32_t));
        }

        ESP_LOGI(TAG, "Confidential; ATAES132A Master Key: 0x"
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                master_key[0], master_key[1], master_key[2],
                master_key[3], master_key[4], master_key[5],
                master_key[6], master_key[7], master_key[8],
                master_key[9], master_key[10], master_key[11],
                master_key[12], master_key[13], master_key[14],
                master_key[15]);

        /* Encrypt Key */
        // todo; replace this; this is an identity placeholder
        uint8_t enc_master_key[16];
        memcpy(enc_master_key, master_key, sizeof(enc_master_key));

        /* Backup Encrypted Key to Device*/
        // Make sure the zone config is in a state where we can write to
        // UserZone 0
        ESP_LOGI(TAG, "Reseting Master UserZone Conifg");
        aes132_reset_master_zoneconfig();
        ESP_LOGI(TAG, "Writing Master Key Backup to UserZone0");
        res = aes132m_write_memory(sizeof(enc_master_key), AES132_USERZONE(0),
                enc_master_key);
        ESP_LOGI(TAG, "Write memory result: %d", res);

        /* Confirm Backup */
        {
            uint8_t rx[16];
            res = aes132m_read_memory(sizeof(enc_master_key), AES132_USERZONE(0),
                    rx);
            ESP_LOGI(TAG, "Read memory result: %d", res);
            ESP_LOGI(TAG, "Confidential; "
                    "ATAES132A Master Key Backup Response: 0x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x"
                    "%02x%02x%02x%02x%02x%02x%02x%02x\n",
                    rx[0], rx[1], rx[2], rx[3], rx[4], rx[5], rx[6], rx[7],
                    rx[8], rx[9], rx[10], rx[11], rx[12], rx[13], rx[14],
                    rx[15]);

            ESP_ERROR_CHECK(memcmp(enc_master_key, rx, sizeof(rx)));
        }
        /* Write Key to Key0 */
        ESP_LOGI(TAG, "Writing Master Key to Key0");
        res = aes132m_write_memory(16, AES132_KEY(0), master_key);
        ESP_LOGI(TAG, "Write key0 result: %d", res);
        
        /* Configure Device */
        ESP_LOGI(TAG, "Configuring Device");
        aes132_write_chipconfig();
        aes132_write_counterconfig();
        aes132_write_keyconfig();
        aes132_write_zoneconfig();
        /* Lock Device */
#ifdef UNIT_TESTING
        // Do Nothing; Don't actually lock device
#else
        //lock_device();
        ESP_LOGE(TAG, "While debugging, if lock_device wasn't commented out, "
                "itd be locked.\n");
        // todo: test
#endif

    }

    sodium_mprotect_noaccess(master_key);
    return 0;
}

uint8_t aes132m_blockread(uint8_t *data, const uint16_t address, const uint8_t count) {
    /* BlockRead Command reads 1~32 bytes of plaintext from userzone or 
     * configuration memory. Standard eeprom read commands can also read userzone
     * data if authentication nor encryption is required. Standard eeprom read
     * commands cannot read configuration memory.
     *
     * Requested data cannot cross page boundaries.
     *
     * Configuration memory can only be read via the blockread command.
     */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_BLOCK_READ;
    mode = 0x00; // Must be zero
    param1 = address;
    param2 = count;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    //todo opcode check
    memcpy(data, &rx_buffer[AES132_RESPONSE_INDEX_DATA], count);
    return 0;
}

/** \brief This function sends data to the device.
 * \param[in] count number of bytes to send
 * \param[in] word_address word address
 * \param[in] data pointer to tx data
 * \return status of the operation
 */
uint8_t aes132m_write_memory(uint8_t count, uint16_t word_address, uint8_t *data)
{
    return aes132c_access_memory(count, word_address, data,  AES132_WRITE);
}


/** \brief This function reads data from the device.
 * \param[in] size number of bytes to read
 * \param[in] word_address pointer to word address
 * \param[out] data pointer to rx data
 * \return status of the operation
*/
uint8_t aes132m_read_memory(uint8_t size, uint16_t word_address, uint8_t *data)
{
    return aes132c_access_memory(size, word_address, data, AES132_READ);
}


/** \brief This function creates a command packet, sends it, and receives its response.
 *         The caller has to allocate enough space for txBuffer and rxBuffer so that
 *         the generated command and the expected response respectively do not overflow
 *         these buffers.
 *
 * \param[in] op_code command op-code
 * \param[in] mode command mode
 * \param[in] param1 first parameter
 * \param[in] param2 second parameter
 * \param[in] datalen1 number of bytes in first data block
 * \param[in] data1 pointer to first data block
 * \param[in] datalen2 number of bytes in second data block
 * \param[in] data2 pointer to second data block
 * \param[in] datalen3 number of bytes in third data block
 * \param[in] data3 pointer to third data block
 * \param[in] datalen4 number of bytes in fourth data block
 * \param[in] data4 pointer to fourth data block
 * \param[in] tx_buffer pointer to command buffer
 * \param[out] rx_buffer pointer to response buffer
 * \return status of the operation
 */
uint8_t aes132m_execute(uint8_t op_code, uint8_t mode, uint16_t param1, uint16_t param2,
            uint8_t datalen1, uint8_t *data1, uint8_t datalen2, uint8_t *data2,
            uint8_t datalen3, uint8_t *data3, uint8_t datalen4, uint8_t *data4,
            uint8_t *tx_buffer, uint8_t *rx_buffer)
{
    uint8_t *p_buffer;
    uint8_t len;

    // Assemble command.
    len = datalen1 + datalen2 + datalen3 + datalen4 + AES132_COMMAND_SIZE_MIN;
    p_buffer = tx_buffer;
    *p_buffer++ = len;
    *p_buffer++ = op_code;
    *p_buffer++ = mode;
    *p_buffer++ = param1 >> 8;
    *p_buffer++ = param1 & 0xFF;
    *p_buffer++ = param2 >> 8;
    *p_buffer++ = param2 & 0xFF;

    if (datalen1 > 0) {
        memcpy(p_buffer, data1, datalen1);
        p_buffer += datalen1;
    }
    if (datalen2 > 0) {
        memcpy(p_buffer, data2, datalen2);
        p_buffer += datalen2;
    }
    if (datalen3 > 0) {
        memcpy(p_buffer, data3, datalen3);
        p_buffer += datalen3;
    }
    if (datalen4 > 0) {
        memcpy(p_buffer, data4, datalen4);
        p_buffer += datalen4;
    }

    // Send command and receive response.
    return aes132c_send_and_receive(&tx_buffer[0], AES132_RESPONSE_SIZE_MAX,
                &rx_buffer[0], AES132_OPTION_DEFAULT);
}

/* Populates the output buffer with n_bytes of random numbers */
uint8_t aes132m_rand(uint8_t *out, const size_t n_bytes) {
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};

    size_t out_head = 0; // Current index into out buffer
    size_t n_copy = 0; // number of bits top copy from rand buffer
    do {
        res = aes132m_execute(AES132_RANDOM, 0x02, 0x0000, 0x0000,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
        if( AES132_FUNCTION_RETCODE_SUCCESS != res ) {
            // todo error handling
            return res;
        }
        if( AES132_DEVICE_RETCODE_SUCCESS != rx_buffer[AES132_RESPONSE_INDEX_RETURN_CODE] ) {
            // todo error handling
            return rx_buffer[AES132_RESPONSE_INDEX_RETURN_CODE]; // this probably isn't the correct thing to return.
        }
        // Copy over up to 16 bytes
        if( n_bytes - out_head > AES132_RAND_BYTE_LEN ) {
            n_copy = AES132_RAND_BYTE_LEN;
        }
        else {
            n_copy = n_bytes - out_head;
        }
        memcpy(&out[out_head], &rx_buffer[AES132_RESPONSE_INDEX_DATA], n_copy);
        out_head += n_copy;
    } while( n_copy > 0 );

    return 0; // success
}

uint8_t aes132m_nonce(uint8_t out_random, const uint8_t *in_seed) {
    /* Random is only populated if in_seed is NULL.
     * Both inputs can be in_seed.
     *
     * random needs to be able to hold a 16-byte output.
     * in_seed should be 12 bytes.
     * */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};

    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t data[12] = { 0 };

    cmd = AES132_NONCE;
    if( NULL == in_seed) {
        mode = 0x01; // Use the inSeed as Nonce
    }
    else {
        mode = 0x00; // Generate a random Nonce using internal RNG
    }
    mode |= AES132_EEPROM_RNG_UPDATE_BIT;
    param1 = 0x0000; // Always 0
    param2 = 0x0000; // Always 0
    if( NULL != in_seed ) {
        memcpy(data, in_seed, sizeof(data));
    }
    res = aes132m_execute(cmd, mode, param1, param2,
            12, data, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        ESP_LOGI(TAG, "Nonce returncode: %02X",
                rx_buffer[AES132_RESPONSE_INDEX_RETURN_CODE]);
    }
    if( NULL != out_random && NULL != in_seed ) {
        memcpy(out_random, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
    }

    return res;
}

uint8_t aes132m_mac_count(uint8_t *count) {
    /* Gets the maccount */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_INFO;
    mode = 0x00;
    param1 = 0x0000;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
        0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( AES132_FUNCTION_RETCODE_SUCCESS == res ) {
        *count = rx_buffer[AES132_RESPONSE_INDEX_DATA + 1];
    }
    else {
    }
    return res;
}

/* Nonce Synchronization - Page 58
 * 1) The Random Command is executed on Device A with Mode<2> set to 1b. 
 *    The first 12 bytes of the random field value in the response are stored 
 *    for use in Step 2.
 * 2) The Nonce command is executed on Device B with Mode<1> set to 1b. The 12-byte random number generated in Step 1 is used as the Nonce command InSeed field value. The 12-byte random field value in the response is stored for use in Step 3.
 * 3) The NonceCompute command is executed on Device A using the 12-byte random number generated in Step 2 as the RandomSeed field value.
 * 
 */
uint8_t aes132m_nonce_sync(uint8_t *nonce_out) {
    /* Used so the ESP32 can compute valid MAC 
     * Populates 12 bytes into nonce_out if nonce_out not null*/
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};

    uint8_t cmd = 0;
    uint8_t mode = 0;
    uint16_t param1 = 0;
    uint16_t param2 = 0;

    /* Step 1: Generate rand 12-byte value; return it and store it in NonceReg
     * Mode<7-3> - 00000b = Reserved; must be 0
     * Mode<2> - 1b = Store the first 12 bytes of the random number in the 
     *                Nonce Register, and return the 16-byte random number. 
     * Mode<1> - 1b = Generate random number using the existing RNG Seed. 
     * Mode<0> - 0b = Reserved; must be 0.
     */
    cmd = AES132_RANDOM;
    mode = 0x06;
    param1 = 0x0000;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
        0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    memcpy(nonce, &rx_buffer[AES132_RESPONSE_INDEX_DATA], sizeof(nonce));
    ESP_LOGD(TAG, "Nonce Reg set to %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X %.2X",
            nonce[0], nonce[1], nonce[2], nonce[3], nonce[4], nonce[5],
            nonce[6], nonce[7], nonce[8], nonce[9], nonce[10], nonce[11]);
    sodium_memzero(tx_buffer, sizeof(tx_buffer));
    sodium_memzero(rx_buffer, sizeof(rx_buffer));

    /* Step 2: Get the MacCount via INFO command
     * Mode<7-0> - 00000000b = Reserved; must be 0.
     * Param1 - 0x0000 = Return MacCount Register
     * Param2 - 0x0000 = Reserved; must be 0.
     */
    cmd = AES132_INFO;
    mode = 0x00;
    param1 = 0x0000;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
        0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    mac_count = rx_buffer[AES132_RESPONSE_INDEX_DATA + 1];
    sodium_memzero(tx_buffer, sizeof(tx_buffer));
    sodium_memzero(rx_buffer, sizeof(rx_buffer));

    if( NULL != nonce_out ) {
        memcpy(nonce_out, nonce, sizeof(nonce));
    }

    return 0; // success
}

uint8_t aes132m_local_auth_compute(uint8_t *out_mac, uint8_t *key,
        const uint8_t *block1, const uint8_t *block2) {
    /* Computes the MAC code locally; does not interact with device.
     * key is a pointer to 128-bits
     *
     * WARNING: This function is ripe for side-channel attacks.
     * DO NOT allow information about the key to leak.
     *
     * probably deprecate this
     * */
    uint8_t b0[16] = { 0 };
    uint8_t bp0[16] = { 0 };
    uint8_t b1[16] = { 0 };
    uint8_t bp1[16] = { 0 };
    uint8_t a0[16] = { 0 };
    uint8_t ap0[16] = { 0 };

    mac_incr();

    //create_b0(b0, data_len);
    // todo: compute bp0 in CBC mode
    //create_b1(bp0, b1, data_len, data);
    // todo: compute bp1 in CBC mode
    create_a0(a0);
    // todo: compute ap0 in CTR mode
    for(uint8_t i=0; i<16; i++){
        out_mac[i] = ap0[i] ^ bp1[i];
    }

    return 0;
}

uint8_t aes132m_key_load(uint128_t key, const uint8_t key_id) {
    /* We use this to load in the PIN authorization key */

    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;
    uint8_t in_mac[16];
    uint8_t enc_key[16];

    /* Configuration - Page 51
     * Data1 - 16 Bytes - Integrity MAC for the input data
     * Data2 - 16 Bytes - Encrypted Key Value
     *
     */
    cmd = AES132_KEY_LOAD;
    mode = 0x01;
    mode |= AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = 0x0000; // only used for VolatileKey
    // todo: encrypt key
    // todo: compute MAC
    res = aes132m_execute(cmd, mode, param1, param2,
            sizeof(in_mac), in_mac,
            sizeof(enc_key), enc_key,
            0, NULL, 0, NULL, tx_buffer, rx_buffer);

    return 0; // success
}

static uint8_t lin2bin(uint8_t bin) {
    /* Used in Counter command; counts number of zero bits from lsb */
    uint8_t count = 0;
    while( !(bin & 0x1) ) {
        count++;
        bin >>= 1;
    }
    return count;
}

uint8_t aes132m_counter(uint32_t *count, uint8_t counter_id) {
    /* MVP
     * Todo:
     *     * error handling
     *     * robust unit tests
     */
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = {0};
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = {0};
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_COUNTER;
    mode = 0x01; // Read the Counter
    mode |= AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = counter_id;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);

    // Interpretting the counter
    uint8_t lin_count, count_flag;
    uint16_t bin_count;
    lin_count = rx_buffer[AES132_RESPONSE_INDEX_DATA];
    count_flag = rx_buffer[AES132_RESPONSE_INDEX_DATA+1];
    bin_count = rx_buffer[AES132_RESPONSE_INDEX_DATA+2]<<8 \
                | rx_buffer[AES132_RESPONSE_INDEX_DATA+3];
    *count = (bin_count*32) + (count_flag/2)*8 + lin2bin(lin_count);
    return 0; // success
}

uint8_t aes132m_key_create(uint8_t key_id) {
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;

    mac_incr();
    cmd = AES132_KEY_CREATE;
    mode = 0x07; 
    mode |= AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = 0x0000;
    res = aes132m_execute(cmd, mode, param1, param2,
            0, NULL, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( res ) {
        if( I2C_FUNCTION_RETCODE_NACK == res ) {
            ESP_LOGE(TAG, "KeyCreate Nack!");
        }
        printf("KeyCreate TX Buffer: \n");
        for(uint8_t i=0; i<sizeof(tx_buffer); i++) {
            printf("%02X ", tx_buffer[i]);
        }
        printf("\n");

        printf("KeyCreate RX Buffer: \n");
        for(uint8_t i=0; i<sizeof(rx_buffer); i++) {
            printf("%02X ", rx_buffer[i]);
        }
        printf("\n");
    }
    return res;
}

uint8_t aes132m_encrypt(const uint8_t *in, uint8_t len, uint8_t key_id,
        uint8_t *out_data, uint8_t *out_mac) {
    /* Encrypts upto 32 bytes of data (*in with length len) using key_id
     * returns cipher text (*out)
     * len must be <=32.
     * out_data must be able to handle 16 bytes (<=16byte in) or 
     * 32 bytes (<=32 byte in)
     * todo: something with the output MAC*/
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;

    uint8_t ciphertext_len;
    if( len > 32 ) {
        ESP_LOGE(TAG, "ENCRYPT accepts a maximum of 32 bytes. "
                "%d bytes were provided.", len);
        return AES132_DEVICE_RETCODE_PARSE_ERROR; // todo: better error
    }
    else if( len > 16 ) {
        ciphertext_len = 32;
    }
    else if(len > 0) {
        ciphertext_len = 16;
        return AES132_DEVICE_RETCODE_PARSE_ERROR; // todo: better error
    }
    else{
        ESP_LOGE(TAG, "No cleartext data provided to encrypt");
        return AES132_DEVICE_RETCODE_PARSE_ERROR; // todo: better error
    }

    mac_incr(); // Encrypt command returns a MAC, increase local mac_count

    cmd = AES132_ENCRYPT;
    mode = 0; // We don't care about the MAC, we want speed
    //mode = AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = len;
    res = aes132m_execute(cmd, mode, param1, param2,
            len, in, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    if( 0 == res ) {
        if( NULL != out_mac ) {
            memcpy(out_mac, &rx_buffer[AES132_RESPONSE_INDEX_DATA], 16);
        }

        if( NULL != out_data ) {
            memcpy(out_data, &rx_buffer[AES132_RESPONSE_INDEX_DATA+16],
                    ciphertext_len);
        }
        else {
            ESP_LOGE(TAG, "Cannot copy ciphertext to NULL pointer");
        }
    }
    return res;
}

#if 0
uint8_t aes132m_decrypt(const uint8_t *in, uint8_t len, uint8_t key_id,
        uint8_t *out) {
    /* Encrypts upto 32 bytes of data (*in with length len) using key_id
     * returns cipher text (*out)
     * len must be <=32 
     * todo: something with the output MAC*/
    uint8_t res;
    uint8_t tx_buffer[AES132_COMMAND_SIZE_MAX] = { 0 };
    uint8_t rx_buffer[AES132_RESPONSE_SIZE_MAX] = { 0 };
    uint8_t cmd, mode;
    uint16_t param1, param2;

    cmd = AES132_ENCRYPT;
    mode = AES132_INCLUDE_SMALLZONE_SERIAL_COUNTER; // MAC Params
    param1 = key_id;
    param2 = len;
    res = aes132m_execute(cmd, mode, param1, param2,
            len, in, 0, NULL, 0, NULL, 0, NULL, tx_buffer, rx_buffer);
    return res;
}
#endif
