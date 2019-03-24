/**
 * @file unaligned.h
 * @brief Convenience functions for unaligned load/store
 * @author Brian Pugh
 */

#include <stdint.h>
#include <stddef.h>

void unalignedCpy(void *dest,void *src,size_t n);
void unalignedSet32(void *dest,uint32_t value);
uint32_t unalignedGet32(void *src);
void unalignedSet8(void *dest,uint8_t value);
uint8_t unalignedGet8(void *src);
