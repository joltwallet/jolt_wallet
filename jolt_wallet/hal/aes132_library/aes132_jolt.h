#ifndef __AES132_JOLT_H__
#define __AES132_JOLT_H__

uint8_t aes132_jolt_setup();

uint8_t aes132_create_stretch_key();
uint8_t aes132_stretch(uint8_t *data, const uint8_t data_len, uint32_t n_iter );

#endif
