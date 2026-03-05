#ifndef LIB_MU_H
#define LIB_MU_H

#include <stddef.h>
#include <stdint.h>
#define SIMD_INIT_SUCCESS   (1<<5)

// Memory manipulation API
void* memset(void* s, uint8_t c, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);
uint8_t enable_simd();
void hcf();

#endif
