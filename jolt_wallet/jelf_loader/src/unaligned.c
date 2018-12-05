#include "unaligned.h"

/* Get a uint8_t at address src */
uint8_t unalignedGet8(void* src) {
    uintptr_t csrc = (uintptr_t)src;
    uint32_t v = *(uint32_t*)(csrc & 0xfffffffc);
    v = (v >> (((uint32_t)csrc & 0x3) * 8)) & 0x000000ff; 
    return v;
}

/* Set a uint8_t value at address */
void unalignedSet8(void* dest, uint8_t value) {
    uintptr_t cdest = (uintptr_t)dest;
    uint32_t d = *(uint32_t*)(cdest & 0xfffffffc);
    uint32_t v = value;
    v = v << ((cdest & 0x3) * 8);
    d = d & ~(0x000000ff << ((cdest & 0x3) * 8));
    d = d | v;
    *(uint32_t*)(cdest & 0xfffffffc) = d;
}

/* Get a uint32_t from address src 
 *    * This is a naive implementation that relies on unalignedGet8
 *    * Could be further optimized
 */
uint32_t unalignedGet32(void* src) {
    uint32_t d = 0;
    uintptr_t csrc = (uintptr_t)src;
    for(int n = 0; n < 4; n++) {
        uint32_t v = unalignedGet8((void*)csrc);
        v = v << (n * 8);
        d = d | v;
        csrc++;
    }
    return d;
}

/* Sets a uint32_t at address src 
 *    * This is a naive implementation that relies on unalignedSet8
 *    * Could be further optimized
 */
void unalignedSet32(void* dest, uint32_t value) {
    uintptr_t cdest = (uintptr_t)dest;
    for(int n = 0; n < 4; n++) {
        unalignedSet8((void*)cdest, value & 0x000000ff);
        value = value >> 8;
        cdest++;
    }
}

/* Copies n bytes from src to dest.
 *     * Operates on single bytes
 *     * Could be further optimized
 */
void unalignedCpy(void* dest, void* src, size_t n) {
    uintptr_t csrc = (uintptr_t)src;
    uintptr_t cdest = (uintptr_t)dest;
    while(n > 0) {
        uint8_t v = unalignedGet8((void*)csrc);
        unalignedSet8((void*)cdest, v);
        csrc++;
        cdest++;
        n--;
    }
}

