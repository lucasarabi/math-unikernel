#ifndef LIB_MU_H
#define LIB_MU_H

#include <stddef.h>
#include <stdint.h>

void* memset(void* s, uint8_t c, size_t n);
void* memcpy(void* restrict dest, const void* restrict src, size_t n);
void hcf(void);

#endif
