#ifndef __AES132_JOLT_H__
#define __AES132_JOLT_H__

/* Should be first thing called after setting up i2c */
uint8_t aes132_jolt_setup();

/* Creates a new key in the stretch slot. Will cause loss of mnemonic */
uint8_t aes132_create_stretch_key();

/* Takes and returns data, encrypted n_iter times by the key in the stretch 
 * slot */
uint8_t aes132_stretch(uint8_t *data, const uint8_t data_len, uint32_t n_iter);

uint8_t aes132_auth(uint8_t *key, uint16_t key_id);
#endif
