#include "unaligned.h"

/* In binary, all 1's except the lsb's to nearest whole address */
//const static uintptr_t round_mask = ((uintptr_t)-sizeof(uintptr_t));
const static uintptr_t round_mask = ((uintptr_t)-4);
//const static uintptr_t shift_mask = (sizeof(uintptr_t)-1);
const static uintptr_t shift_mask = 3;

/* Get a uint8_t at address src */
uint8_t unalignedGet8(void* src) {
    //printf("unalignedGet8 %p\n", src);
    uintptr_t csrc = (uintptr_t)src;
    //printf("rounded address: %p\n", (csrc & round_mask));
    //printf("uintptr_t size: %d\n", sizeof(uintptr_t));
    //printf("round_mask: %x\n", round_mask);
    //printf("shift_mask: %x\n", shift_mask);
    uint32_t v = *(uint32_t*)(csrc & round_mask);
    v = (v >> ((csrc & shift_mask) * 8)) & 0xff; 
    return v;
}

/* Set a uint8_t value at address */
void unalignedSet8(void* dest, uint8_t value) {
    //printf("unalignedSet8 %p\n", dest);
    uintptr_t cdest = (uintptr_t)dest;
    uint32_t d = *(uint32_t*)(cdest & round_mask);
    uint32_t v = value;
    v = v << ((cdest & (sizeof(uintptr_t)-1)) * 8);
    d = d & ~(0xff << ((cdest & shift_mask) * 8));
    d = d | v;
    *(uint32_t*)(cdest & round_mask) = d;
}

/* Get a uint32_t from address src 
 *    * This is a naive implementation that relies on unalignedGet8
 *    * Could be further optimized
 */
uint32_t unalignedGet32(void* src) {
    //printf("unalignedGet32 %p; uint32 value: %X\n", src, *(uint32_t*)src);
    uint32_t d = 0;
    uintptr_t csrc = (uintptr_t)src;
    for(int n = 0; n < 4; n++) {
        uint32_t v = unalignedGet8((void*)csrc);
        v = v << (n * 8);
        d = d | v;
        csrc++;
    }
    //printf("unalignedGet32 returning %X\n", d);
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

