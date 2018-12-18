#ifndef JOLT_SYSCORE_DECOMPRESS_H
#define JOLT_SYSCORE_DECOMPRESS_H

int decompress_hs_file(char *src_fn);
int decompress_fd_to_fd(FILE *compressed, FILE *decompressed);
int decompress_fd_to_alloc_mem(FILE *fd, uint8_t *mem, const size_t mem_len);
uint8_t *decompress_fn_to_mem(char *fn);
size_t decompress_check_size(FILE *fd);

#endif
